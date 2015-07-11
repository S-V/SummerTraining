/*
=============================================================================
	File:	Serialization.cpp
	Desc:	Binary serialization of arbitrary types via reflection.
	ToDo:
		add compatibility checks;
		don't use a pointer patch table, use a linked list of pointer offsets?
		write MAKEFOURCC('P','N','T','R'); in place of pointers for debugging?
	Useful references:
	The serialization library FlatBuffers:
	http://google.github.io/flatbuffers/
	Fast mmap()able data structures
	https://deplinenoise.wordpress.com/2013/03/31/fast-mmapable-data-structures/
=============================================================================
*/
#include <Core/Core_PCH.h>
#pragma hdrstop
#include <Base/Util/FourCC.h>
#include <Core/Asset.h>
#include <Core/Serialization.h>
#include <Core/Util/ScopedTimer.h>

#define DEBUG_SERIALIZATION (0)

#if 1
	#define DBG_MSG(...)
	#define DBG_MSG2(ctx,...)
#else
	#define DBG_MSG(...)\
		DBGOUT(__VA_ARGS__)
	#define DBG_MSG2(ctx,...)\
		LogStream(LL_Info).Repeat(' ',ctx.depth).PrintF(__VA_ARGS__);
#endif


namespace Serialization
{
	using namespace Reflection;

#if MX_DEBUG
	static const UINT32 PADDING_VALUE = MCHAR4('P','A','D','N');
#else
	static const UINT32 PADDING_VALUE = 0;
#endif

	static const UINT32 NULL_POINTER_OFFSET = ~0UL;

	enum { OBJECT_BLOB_ALIGNMENT = 16 };

	//
	// Memory image serialization and in-place loading
	// NOTE: all pointer offsets are relative to start of object data
	// (object data starts right after the header).

	// each chunk represents a contiguous memory block
	struct SChunk
	{
		const char *name;	// for debugging
		const void *data;	// pointer to the start of the memory block		
		UINT32		size;	// unaligned size of this chunk
		UINT32		offset;	// (aligned) file offset this chunk begins at (0 for the first chunk)
		UINT32		alignment;	// alignment requirement for the memory block of this chunk
	};
	// represents a pointer that needs to be fixed-up during in-place loading
	struct SPointer
	{
		const void *address;	// memory address of this pointer itself
		const void *target;		// memory address this pointer points at
		const char *name;		// for debugging
	};
	struct STypeInfo
	{
		SClassId *	o;
	};

	static inline bool ContainsAddress( const SChunk& chunk, const void* pointer )
	{
		mxASSERT_PTR(pointer);
		return mxPointerInRange( pointer, chunk.data, chunk.size );
	}
	static inline UINT32 GetFileOffset( const void* pointer, const SChunk& chunk )
	{
		mxASSERT(ContainsAddress(chunk, pointer));
		const UINT32 relativeOffset = mxGetByteOffset32( chunk.data, pointer );
		const UINT32 absoluteOffset = chunk.offset + relativeOffset;
		return absoluteOffset;
	}
	static UINT32 GetFileOffset( const void* pointer, const TArray< SChunk >& chunks )
	{
		mxASSERT(pointer != NULL);
		for( UINT32 iChunk = 0; iChunk < chunks.Num(); iChunk++ )
		{
			const SChunk &	chunk = chunks[ iChunk ];
			const ptrdiff_t relativeOffset = (char*)pointer - (char*)chunk.data;
			if( relativeOffset >= 0 && relativeOffset < chunk.size )
			{
				return chunk.offset + relativeOffset;
			}
		}
		ptERROR("Bad pointer: 0x%p\n", pointer);
		return NULL_POINTER_OFFSET;
	}

	// Gathers information necessary for memory image serialization: collects all memory blocks and pointers.
	struct LIPInfoGatherer : public Reflection::AVisitor2
	{mxOPTIMIZE("reduce dynamic memory allocations in these arrays:")
		TArray< SChunk > 	chunks;		// memory blocks to be serialized
		TArray< SPointer > 	pointers;	// pointers to be patched after loading; they can only point inside the above memory blocks
		TArray< STypeInfo > typeFixups;	// references to type IDs (serialized as TypeGUIDs)
		TArray< AssetID* > 	assetIdFixups;
	public:
		const SChunk* FindChunk( const void* _memory ) const
		{
			for( UINT32 i = 0; i < chunks.Num(); i++ )
			{
				const SChunk& chunk = chunks[ i ];
				if( chunk.data == _memory ) {
					return &chunk;
				}
			}
			return NULL;
		}
		//const SPointer* FindPointer( const void* _target ) const
		//{
		//	for( UINT32 i = 0; i < pointers.Num(); i++ )
		//	{
		//		const SPointer& pointer = pointers[ i ];
		//		if( pointer.target == _target ) {
		//			return &pointer;
		//		}
		//	}
		//	return NULL;
		//}
		SChunk& AddChunk( const void* _start, UINT32 _length, UINT32 _alignment, const Context& _ctx )
		{
			const char* _name = _ctx.GetMemberName();
			DBG_MSG2(_ctx,"AddChunk(): start=0x%p, length=%u, align=%u (\'%s\')", _start, _length, _alignment, _name);
			mxASSERT_PTR(_start);
			mxASSERT(_length > 0);
			_alignment = largest(_alignment, MINIMUM_ALIGNMENT);
			SChunk & newChunk = chunks.Add();
			{
				newChunk.name = _name;
				newChunk.data = _start;
				newChunk.size = _length;
				newChunk.offset = ~0UL;	// file offset will be resolved after collecting all chunks
				newChunk.alignment = _alignment;
			}
			return newChunk;
		}
		SPointer& AddPointer( const void* _address, const void* _target, const Context& _ctx )
		{
			const char* _name = _ctx.GetMemberName();
			DBG_MSG2(_ctx,"AddPointer() at 0x%p to 0x%p (\'%s\')", _address, _target, _name);
			SPointer &	newPointer = pointers.Add();
			newPointer.address = _address;
			newPointer.target = _target;
			newPointer.name = _name;
			return newPointer;
		}
		// returns the total size of all memory blocks
		UINT32 ResolveChunkOffsets()
		{
			// Sort memory blocks as needed (e.g. by increasing addresses to slightly improve data locality, etc.).
			// ...

			// Calculate absolute file offsets of all memory blocks
			// and the total size of the serialized memory image.
			UINT32 offset = 0;
			for( UINT32 iChunk = 0; iChunk < chunks.Num(); iChunk++ )
			{
				SChunk & chunk = chunks[ iChunk ];
				offset = AlignUp( offset, chunk.alignment );
				chunk.offset = offset;
				offset += chunk.size;
				DBG_MSG("WRITE: Chunk '%s': %u bytes at %u", chunk.name, chunk.size, chunk.offset);
			}
			offset = AlignUp(offset,OBJECT_BLOB_ALIGNMENT);
			return offset;
		}
		UINT32 WriteChunksAndFixUpTables( AStreamWriter &_stream )
		{
			// Write all memory blocks to file.
			UINT32 bytesWritten = 0;	//<= not including size of blob header
			for( UINT32 iChunk = 0; iChunk < chunks.Num(); iChunk++ )
			{
				const SChunk & chunk = chunks[ iChunk ];
				const UINT32 currentOffset = bytesWritten;
				const UINT32 alignedOffset = chunk.offset;
				//mxASSERT2(IsAlignedBy(alignedOffset, chunk.alignment), "data must start at aligned offset");
				const UINT32 sizeOfPadding = alignedOffset - currentOffset;
				if( sizeOfPadding > 0 ) {
					Write_N_bytes( _stream, PADDING_VALUE, sizeOfPadding );
				}
				_stream.Write( chunk.data, chunk.size );
				bytesWritten += (chunk.size + sizeOfPadding);
			}

			{
				const UINT32 currentOffset = bytesWritten;
				const UINT32 alignedOffset = AlignUp( currentOffset, OBJECT_BLOB_ALIGNMENT );
				const UINT32 sizeOfPadding = alignedOffset - currentOffset;
				Write_N_bytes( _stream, PADDING_VALUE, sizeOfPadding );
				bytesWritten = alignedOffset;
			}

			// Relocation data begin starts right after serialized object data.
			const UINT32 relocationTableOffset = bytesWritten;

			// Append pointer patch tables.
			const UINT32 numPointerFixups = pointers.Num();
			_stream << numPointerFixups;
			for( UINT32 i = 0; i < numPointerFixups; i++ )
			{
				const SPointer& pointer = pointers[ i ];
				const UINT32 pointerOffset = GetFileOffset( pointer.address, chunks );
				const UINT32 targetOffset = GetFileOffset( pointer.target, chunks );
				_stream << pointerOffset;
				_stream << targetOffset;
				DBG_MSG("WRITE: Pointer '%s': %u -> %u", pointer.name, pointerOffset, targetOffset);
			}
			bytesWritten += numPointerFixups * (sizeof(UINT32) * 2);

			const UINT32 numTypeFixups = typeFixups.Num();
			_stream << numTypeFixups;
			for( UINT32 i = 0; i < numTypeFixups; i++ )
			{
				const STypeInfo& pointer = typeFixups[ i ];
				const UINT32 pointerOffset = GetFileOffset( pointer.o, chunks );
				const UINT32 typeID = pointer.o->type->GetTypeID();
				_stream << pointerOffset;
				_stream << typeID;
			}
			bytesWritten += numTypeFixups * (sizeof(UINT32) * 2);

			const UINT32 numAssetIdFixups = assetIdFixups.Num();
			_stream << numAssetIdFixups;
			for( UINT32 iAssetRef = 0; iAssetRef < numAssetIdFixups; iAssetRef++ )
			{
				const AssetID* assetID = assetIdFixups[ iAssetRef ];

				const UINT32 pointerOffset = GetFileOffset( assetID, chunks );
				_stream << pointerOffset;
				bytesWritten += sizeof(pointerOffset);

				WriteAssetID( *assetID, _stream );

				const UINT32 realLength = assetID->d.size();
				const UINT32 alignedLength = TAlignUp< String::ALIGNMENT >( realLength );
				bytesWritten += sizeof(realLength);
				if( realLength > 0 ) {
					bytesWritten += alignedLength;
				}
			}

			DBG_MSG("WRITE: %u chunks, %u pointers, %u typeIDs, %u assetIDs at %u (totalsize=%u,tablesize=%u)",
				chunks.Num(),pointers.Num(),typeFixups.Num(),assetIdFixups.Num(),relocationTableOffset,bytesWritten,bytesWritten-relocationTableOffset);

			return relocationTableOffset;
		}
	public:
		typedef Reflection::AVisitor Super;

		LIPInfoGatherer()
		{}
		//-- Reflection::AVisitor
		virtual void Visit_Pointer( VoidPointer& p, const mxPointerType& type, const Context& _context ) override
		{
			// null pointers will be written as it is - zeros
			if( p.o != NULL )
			{
				//if( !this->FindPointer( &p.o ) )
				{
					this->AddPointer( &p.o, p.o, _context );
				}
				//else {
				//	DBG_MSG("Skip pointer at 0x%p to 0x%p (\'%s\')", &p.o, p.o, _context.GetMemberName());
				//}
			}
		}
		virtual void Visit_TypeId( SClassId * o, const Context& _context ) override
		{
			DBG_MSG2(_context,"Visit_TypeId(): '%s' at 0x%p (\'%s\')", o->type->GetTypeName(), o->type, _context.GetMemberName());
			STypeInfo & newItem = typeFixups.Add();
			newItem.o = o;
		}
		virtual bool Visit_Array( void * _array, const mxArray& _type, const Context& _context ) override
		{
			const UINT32 capacity = _type.Generic_Get_Capacity( _array );
			if( capacity > 0 )
			{
				const void* arrayBase = _type.Generic_Get_Data( _array );
				mxASSERT_PTR(arrayBase);
				const mxType& itemType = _type.m_itemType;
				if( _type.IsDynamic() )
				{
					this->AddPointer( _type.Get_Array_Pointer_Address( _array ), arrayBase, _context );
					this->AddChunk( arrayBase, capacity * itemType.m_size, itemType.m_align, _context );
				}
			}
			const bool bIterateOverElements = !Type_Is_Bitwise_Serializable( _type.m_itemType.m_kind );
			return bIterateOverElements;
		}
		virtual void Visit_String( String & _string, const Context& _context ) override
		{
			if( _string.NonEmpty() )
			{
				this->AddChunk( _string.ToPtr(), _string.Length()+1, String::ALIGNMENT, _context );
				this->AddPointer( _string.GetBufferAddress(), _string.ToPtr(), _context );
			}
		}
		virtual void Visit_AssetId( AssetID & _assetId, const Context& _context )
		{
			assetIdFixups.Add( &_assetId );
		}
	};

	ERet SaveImage( const void* _o, const mxClass& _type, AStreamWriter &_stream )
	{
		LIPInfoGatherer	lip;

		Reflection::AVisitor2::Context	ctx;

		// Add the root object body.
		lip.AddChunk( _o, _type.m_size, _type.m_align, ctx );

		// Recursively visit all referenced objects.
		Reflection::Walker2::Visit( const_cast<void*>(_o), _type, &lip );

		// Determine file offsets of all memory blocks.
		const UINT32 alignedDataSize = lip.ResolveChunkOffsets();

		// Write the header.
		ImageHeader	header;
		{
			header.session = PtSessionInfo::CURRENT;
			header.classId = _type.GetTypeID();
			header.payload = alignedDataSize;
		}
		mxDO(_stream.Put( header ));

		DBG_MSG("WRITE: Object: '%s' (%#010x), data size: '%u', table start: '%u'",
			_type.GetTypeName(), header.classId, alignedDataSize, sizeof(header) + alignedDataSize);

		// Write all memory blocks and relocation tables.
		lip.WriteChunksAndFixUpTables( _stream );

		return ALL_OK;
	}

	static ERet ReadAndApplyFixups( AStreamReader& _reader, void* _objectBuffer, UINT32 _bufferSize )
	{
		// Load and apply fixup tables.
		// Relocate pointers.
		UINT32 numPointerFixups;
		_reader.Get(numPointerFixups);
		for( UINT32 i = 0; i < numPointerFixups; i++ )
		{
			UINT32 pointerOffset;
			UINT32 targetOffset;
			mxDO(_reader.Get(pointerOffset));
			mxDO(_reader.Get(targetOffset));

			void* pointerAddress = mxAddByteOffset( _objectBuffer, pointerOffset );
			void* targetAddress = mxAddByteOffset( _objectBuffer, targetOffset );
			*((void**)pointerAddress) = targetAddress;
		}
		// Fixup type ids.
		UINT32 numTypeFixups;
		_reader.Get(numTypeFixups);
		for( UINT32 i = 0; i < numTypeFixups; i++ )
		{
			UINT32 pointerOffset;
			UINT32 typeID;
			mxDO(_reader.Get(pointerOffset));
			mxDO(_reader.Get(typeID));

			const mxClass* typeInfo = TypeRegistry::Get().FindClassByGuid( typeID );
			mxASSERT_PTR(typeInfo);
			void* pointerAddress = mxAddByteOffset( _objectBuffer, pointerOffset );
			*(void**)pointerAddress = (void*)typeInfo;
		}
		UINT32 numAssetIdFixups;
		_reader.Get(numAssetIdFixups);
		for( UINT32 i = 0; i < numAssetIdFixups; i++ )
		{
			UINT32 pointerOffset;
			mxDO(_reader.Get(pointerOffset));

			AssetID* assetId = (AssetID*) mxAddByteOffset( _objectBuffer, pointerOffset );
			new(assetId) AssetID();
			mxDO(ReadAssetID( _reader, assetId ));
		}
		return ALL_OK;
	}

	static ERet ReadAndApplyFixups( void* objectBuffer, UINT32 objectDataSize, void* fixupTables, UINT32 tableDataSize )
	{
		MemoryReader	stream( fixupTables, tableDataSize );
		mxDO(ReadAndApplyFixups( stream, objectBuffer, objectDataSize ));
		return ALL_OK;
	}

	template< class HEADER >
	static ERet ValidatePlatformAndType( const HEADER& header, const mxClass& type )
	{
		mxDO(PtSessionInfo::ValidateSession(header.session));
		chkRET_X_IF_NOT(type.GetTypeID() == header.classId, ERR_OBJECT_OF_WRONG_TYPE);
		return ALL_OK;
	}
	template< class HEADER >
	static ERet ValidateSizeAndAlignment( const HEADER& header, const mxClass& type, const void* buffer, UINT32 length )
	{
		chkRET_X_IF_NOT(length >= header.payload, ERR_BUFFER_TOO_SMALL);
		chkRET_X_IF_NOT(type.m_size <= header.payload, ERR_BUFFER_TOO_SMALL);
		chkRET_X_IF_NOT(IsAlignedBy(buffer, type.m_align), ERR_INVALID_ALIGNMENT);
		chkRET_X_IF_NOT(type.GetTypeID() == header.classId, ERR_OBJECT_OF_WRONG_TYPE);
		return ALL_OK;
	}

	ERet LoadImage( AStreamReader& stream, const mxClass& type, ByteArrayT &buffer )
	{
		ImageHeader	header;
		mxDO(stream.Get(header));

		mxDO(ValidatePlatformAndType(header, type));

		DBG_MSG("READ: Object: '%s' (%#010x), data size: '%u', table start: '%u'",
			type.GetTypeName(), header.classId, header.payload, sizeof(header) + header.payload);

		mxDO(buffer.SetNum(header.payload));

		mxDO(ValidateSizeAndAlignment(header, type, buffer.ToPtr(), buffer.Num()));

		mxDO(stream.Read(buffer.ToPtr(), header.payload));

		header.payload = AlignUp( header.payload, OBJECT_BLOB_ALIGNMENT );
		mxDO(ReadAndApplyFixups( stream, buffer.ToPtr(), buffer.Num() ));
		Reflection::MarkMemoryAsExternallyAllocated( buffer.ToPtr(), type );

		return ALL_OK;
	}

	ERet LoadInPlace( const mxClass& type, void* buffer, UINT32 length, void *&o )
	{
		ImageHeader& header = *static_cast< ImageHeader* >( buffer );

		mxDO(ValidatePlatformAndType(header, type));
		mxDO(ValidateSizeAndAlignment(header, type, buffer, length));

		header.payload = AlignUp( header.payload, OBJECT_BLOB_ALIGNMENT );
		void* objectData = mxAddByteOffset(buffer, sizeof(ImageHeader));
		void* fixupsData = mxAddByteOffset(objectData, header.payload);
		UINT32 tableSize = length - sizeof(ImageHeader) - header.payload;

		mxDO(ReadAndApplyFixups(objectData, header.payload, fixupsData, tableSize));
		Reflection::MarkMemoryAsExternallyAllocated( objectData, type );

		o = objectData;

		return ALL_OK;
	}

	ERet LoadInPlace(
		const mxClass& type, const ImageHeader& header,
		void * buffer, UINT32 length,
		AStreamReader& stream
		)
	{
		mxDO(ValidatePlatformAndType(header, type));
		mxDO(ValidateSizeAndAlignment(header, type, buffer, length));

		mxDO(stream.Read( buffer, header.payload ));
		mxDO(ReadAndApplyFixups( stream, buffer, header.payload ));
		Reflection::MarkMemoryAsExternallyAllocated( buffer, type );

		return ALL_OK;
	}

	//
	// Binary serialization
	//

	ERet SaveBinary( const void* o, const mxClass& _type, AStreamWriter &stream )
	{
		BinaryHeader	header;
		{
			header.session = PtSessionInfo::CURRENT;
			header.classId = _type.GetTypeID();
		}
		mxDO(stream.Put(header));


		class BinarySerializer : public Reflection::Visitor3< AStreamWriter& > {
		public:
			BinarySerializer()
			{}
			virtual ERet Visit_Field( void * _memory, const mxField& _field, const Context3& _context, AStreamWriter &_stream )
			{
#if DEBUG_SERIALIZATION
				String256 tmp;
				Str::SPrintF(tmp,"[FIELD: %s (%s)]",_field.name,_field.type.GetTypeName());
				mxDO(tmp.SaveToStream(_stream));
#endif
				mxDO(this->Visit( _memory, _field.type, _context, _stream ));
				return ALL_OK;
			}
			virtual ERet Visit_Array( void * _array, const mxArray& _type, const Context3& _context, AStreamWriter &_stream ) override
			{
#if DEBUG_SERIALIZATION
				String256 tmp;
				Str::SPrintF(tmp,"[ARRAY: %s (%u items, %u bytes)]",_type.GetTypeName(),_type.Generic_Get_Count( _array ),_type.CalculateDataSize(_array));
				mxDO(tmp.SaveToStream(_stream));
#endif

				const UINT32 arrayCount = _type.Generic_Get_Count( _array );
				mxDO(_stream.Put( arrayCount ));

				const mxType& itemType = _type.m_itemType;
				const UINT32 itemStride = itemType.m_size;
				void* arrayBase = _type.Generic_Get_Data( _array );

				const bool isPOD = Type_Is_Bitwise_Serializable( itemType.m_kind );
				if( isPOD ) {
					mxDO(_stream.Write( arrayBase, arrayCount * itemStride ));
				} else {
					mxDO(Reflection::Visit_Array_Elements< AStreamWriter& >( arrayBase, arrayCount, itemType, _context, *this, _stream ));
				}

				return ALL_OK;
			}
			virtual ERet Visit_POD( void * _memory, const mxType& type, const Context3& _context, AStreamWriter &_stream ) override
			{
				mxDO(_stream.Write( _memory, type.m_size ));
				return ALL_OK;
			}
			virtual ERet Visit_String( String * _string, const Context3& _context, AStreamWriter &_stream ) override
			{
				mxDO(_string->SaveToStream( _stream ));
				return ALL_OK;
			}
			virtual ERet Visit_TypeId( SClassId * _pointer, const Context3& _context, AStreamWriter &_stream ) override
			{
				const TypeID typeId = ( _pointer->type != NULL ) ? _pointer->type->GetTypeID() : mxNULL_TYPE_ID;
				mxDO(_stream.Put( typeId ));
				return ALL_OK;
			}
			virtual ERet Visit_AssetId( AssetID * _assetId, const Context3& _context, AStreamWriter &_stream ) override
			{
				mxDO(WriteAssetID( *_assetId, _stream ));
				return ALL_OK;
			}
			virtual ERet Visit_Pointer( void * _pointer, const mxPointerType& _type, const Context3& _context, AStreamWriter &_stream ) override
			{
				return ERR_NOT_IMPLEMENTED;
			}
		};

		BinarySerializer	serializer;
		mxDO(serializer.Visit( const_cast< void* >(o), _type, stream ));

		return ALL_OK;
	}

	ERet LoadBinary( AStreamReader& stream, const mxClass& _type, void *o )
	{
		BinaryHeader	header;
		mxDO(stream.Get(header));

		mxDO(ValidatePlatformAndType(header, _type));

		// Read object data and allocate memory for everything

		class BinaryDeserializer : public Reflection::Visitor3< AStreamReader& > {
		public:
			virtual ERet Visit_Field( void * _memory, const mxField& _field, const Context3& _context, AStreamReader& _stream )
			{
#if DEBUG_SERIALIZATION
				String256 tmp;
				mxDO(tmp.LoadFromStream(_stream));
#endif
				mxDO(this->Visit( _memory, _field.type, _context, _stream ));
				return ALL_OK;
			}
			virtual ERet Visit_Array( void * _array, const mxArray& _type, const Context3& _context, AStreamReader& _stream ) override
			{
#if DEBUG_SERIALIZATION
				String256 tmp;
				mxDO(tmp.LoadFromStream(_stream));
#endif
				UINT32 arrayCount = 0;
				mxDO(_stream.Get( arrayCount ));
				mxASSERT( arrayCount < Reflection::MAX_ARRAY_ELEMENTS );

				mxDO(_type.Generic_Set_Count( _array, arrayCount ));

				void* arrayBase = _type.Generic_Get_Data( _array );

				const mxType& itemType = _type.m_itemType;
				const UINT32 itemStride = itemType.m_size;
				const UINT arrayDataSize = arrayCount * itemStride;
				mxASSERT( arrayDataSize < Reflection::MAX_ARRAY_SIZE_IN_BYTES );

				const bool isPOD = Type_Is_Bitwise_Serializable( itemType.m_kind );
				if( isPOD ) {
					mxDO(_stream.Read( arrayBase, arrayDataSize ));
				} else {
					mxDO(Reflection::Visit_Array_Elements< AStreamReader& >( arrayBase, arrayCount, itemType, _context, *this, _stream ));
				}

				return ALL_OK;
			}
			virtual ERet Visit_POD( void * _memory, const mxType& type, const Context3& _context, AStreamReader &_stream ) override
			{
				mxDO(_stream.Read( _memory, type.m_size ));
				return ALL_OK;
			}
			virtual ERet Visit_String( String * _string, const Context3& _context, AStreamReader &_stream ) override
			{
				mxDO(_string->LoadFromStream( _stream ));
				return ALL_OK;
			}
			virtual ERet Visit_TypeId( SClassId * _pointer, const Context3& _context, AStreamReader &_stream ) override
			{
				TypeID typeId;
				mxDO(_stream.Get( typeId ));
				if( typeId != mxNULL_TYPE_ID ) {
					_pointer->type = TypeRegistry::Get().FindClassByGuid( typeId );
					if( !_pointer->type ) {
						DebugPrint(_context,"No class with id=%d",typeId);
						return ERR_OBJECT_NOT_FOUND;
					}
				}
				return ALL_OK;
			}
			virtual ERet Visit_AssetId( AssetID * _assetId, const Context3& _context, AStreamReader &_stream ) override
			{
				mxDO(ReadAssetID( _stream, _assetId ));
				return ALL_OK;
			}
			virtual ERet Visit_Pointer( void * _pointer, const mxPointerType& _type, const Context3& _context, AStreamReader &_stream ) override
			{
				return ERR_NOT_IMPLEMENTED;
			}
		};
		BinaryDeserializer	deserializer;
		mxDO(deserializer.Visit( const_cast< void* >(o), _type, stream ));

		return ALL_OK;
	}

	ERet SaveBinaryToFile( const void* o, const mxClass& type, const char* file )
	{
		FileWriter	stream( file, FileWrite_NoErrors );
		if( !stream.IsOpen() ) {
			return ERR_FAILED_TO_OPEN_FILE;
		}
		return SaveBinary(o, type, stream);
	}
	ERet LoadBinaryFromFile( const char* file, const mxClass& type, void *o )
	{
		FileReader	reader(file, FileRead_NoErrors);
		if( !reader.IsOpen() ) {
			return ERR_FAILED_TO_OPEN_FILE;
		}
		return LoadBinary(reader, type, o);
	}

	ERet SaveClumpImage( const Clump& _clump, AStreamWriter &_stream )
	{
		LIPInfoGatherer	lip;

		Reflection::AVisitor2::Context	clumpCtx;
		clumpCtx.userName = "Clump";

		// Add the root object body.
		lip.AddChunk( &_clump, sizeof(Clump), EFFICIENT_ALIGNMENT, clumpCtx );

		// Recursively visit all referenced objects.
		Reflection::Walker2::Visit( c_cast(void*)(&_clump), mxCLASS_OF(_clump), &lip, clumpCtx );


		ObjectList::Head currentList = _clump.GetObjectLists();
		while( currentList != NULL )
		{
			const UINT32 objectCount = currentList->Num();
			const mxClass& objectType = currentList->GetType();
			CStruct* objectsArray = currentList->GetArrayPtr();
			const UINT32 arrayStride = currentList->GetStride();

			//DBGOUT("Object list of type '%s':", objectType.GetTypeName());

			Reflection::AVisitor2::Context	objectListCtx( clumpCtx.depth + 3 );
			objectListCtx.userName = objectType.GetTypeName();

			lip.AddChunk( currentList, sizeof(ObjectList), EFFICIENT_ALIGNMENT, objectListCtx );

			Reflection::Walker2::Visit( c_cast(void*)currentList, mxCLASS_OF(*currentList), &lip, objectListCtx );

			lip.AddChunk( objectsArray, objectCount*arrayStride, objectType.m_align, objectListCtx );


			Reflection::AVisitor2::Context	objectCtx( objectListCtx.depth + 1 );
			objectCtx.userName = objectType.GetTypeName();

			ObjectList::IteratorBase it( *currentList );
			while( it.IsValid() )
			{
				void* o = it.ToVoidPtr();
				Reflection::Walker2::Visit( o, objectType, &lip, objectCtx );
				it.MoveToNext();
			}

			currentList = currentList->_next;
		}

		// Determine file offsets of all memory blocks.
		const UINT32 alignedDataSize = lip.ResolveChunkOffsets();

		// Write the header.
		ImageHeader	header;
		{
			header.session = PtSessionInfo::CURRENT;
			header.classId = mxCLASS_OF(_clump).GetTypeID();
			header.payload = alignedDataSize;
		}
		mxDO(_stream.Put( header ));

		DBG_MSG("WRITE: Object: '%s' (%#010x), data size: '%u', table start: '%u'",
			"Clump", header.classId, alignedDataSize, sizeof(header) + alignedDataSize);

		// Write all memory blocks and relocation tables.
		lip.WriteChunksAndFixUpTables( _stream );

		return ALL_OK;
	}

	ERet LoadClumpImage( AStreamReader& _stream, UINT32 _payload, void *_buffer )
	{
		mxDO(_stream.Read( _buffer, _payload ));

		Clump* clump = new(_buffer) Clump();

		// Patch the clump after loading.

		mxDO(ReadAndApplyFixups( _stream, _buffer, _payload ));

		new(&clump->m_objectListsStorage)FreeListAllocator();
		clump->m_objectListsStorage.Initialize( sizeof(ObjectList), 16 );

		TellNotToFreeMemory		markMemoryAsExternallyAllocated;
		clump->IterateObjects( &markMemoryAsExternallyAllocated, NULL );

		return ALL_OK;
	}

}//namespace Serialization

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
