#include <Core/Core_PCH.h>
#pragma hdrstop

#include <Core/Core.h>
#include <Core/Editor.h>
#include <Core/Util/Tweakable.h>

#if MX_EDITOR

namespace TweakUtil
{
	namespace
	{
		struct PrivateData
		{
			PropertyMap	tweakables;
		};
		static TPtr< PrivateData >	gData;
	}//namespace

	TCallback< void (AProperty*) >	OnNewTweakableAdded;
	TCallback< void (void) >		OnShutDown;

	void Setup()
	{
		gData.ConstructInPlace();
	}

	void Close()
	{
		if( OnShutDown ) {
			OnShutDown();
		}
		gData->tweakables.Clear();
		gData.Destruct();
	}

	PropertyMap& GetAllTweakables()
	{
		return gData->tweakables;
	}

/*
	struct TweakableVar
	{
		//
	};
	struct TweakableBool : TweakableVar
	{
		//
	};
	struct TweakableInt : TweakableVar
	{
		//
	};
	struct TweakableFloat : TweakableVar
	{
		//
	};
*/
	struct TweakableVarInfo
	{
		const char*	m_expr;
		const char*	m_file;
		int			m_line;

	public:
		void Init( const char* expr, const char* file, int line )
		{
			m_expr = expr;
			m_file = file;
			m_line = line;
		}
	};

	typedef TGetSetProperty< BooleanProperty, bool, bool, bool, TweakableVarInfo >	TweakableBool;
	typedef TGetSetProperty< FloatProperty, float, double, double, TweakableVarInfo >	TweakableFloat;

	void Tweak_Variable( bool * var, const char* expr, const char* file, int line )
	{
		AProperty* existingEntry = gData->tweakables.FindRef( var );
		if( existingEntry == NULL )
		{
			TweakableBool* newEntry = new TweakableBool( var );
			newEntry->Init( expr, file, line );
			newEntry->name.SetReference(Chars(expr));

			gData->tweakables.Set( var, newEntry );

			if( TweakUtil::OnNewTweakableAdded ) {
				TweakUtil::OnNewTweakableAdded( newEntry );
			}
		}
	}

	void Tweak_Variable( int * var, const char* expr, const char* file, int line )
	{
		UNDONE;
	}

	void Tweak_Variable( float * var, const char* expr, const char* file, int line )
	{
		AProperty* existingEntry = gData->tweakables.FindRef( var );
		if( existingEntry == NULL )
		{
			TweakableFloat* newEntry = new TweakableFloat( var );
			newEntry->Init( expr, file, line );
			newEntry->name.SetReference(Chars(expr));

			gData->tweakables.Set( var, newEntry );

			if( TweakUtil::OnNewTweakableAdded ) {
				TweakUtil::OnNewTweakableAdded( newEntry );
			}
		}
	}

	void Tweak_Variable( double * var, const char* expr, const char* file, int line )
	{
		UNDONE;
	}

}//namespace TweakUtil

#endif // MX_EDITOR

mxNO_EMPTY_FILE

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
