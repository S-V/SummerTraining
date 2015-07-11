/*
=============================================================================
	File:	TextStream.h
	Desc:	
=============================================================================
*/
#pragma once

enum { MAX_SCOPE_DEPTH = 32 };
enum { MAX_INDENT_LEVEL = 32 };

class TextWriter : public TextStream {
public:
	explicit TextWriter( AStreamWriter& stream );
	~TextWriter();

	// writes string with indentation
	virtual void VWrite( const char* text, int length ) override;

	inline void NewLine()
	{
		this->GetStream().Write("\n",1);
	}

	// writes string only, without indentation, doesn't write tabs
	TextWriter& Emit( const char* s );
	TextWriter& Emit( const char* s, UINT length );
	TextWriter& Emitf( const char* fmt, ... );

	void EnterScope();
	void LeaveScope( bool bSemicolon = false );

	// insert whitespaces for nicer indentation and formatting;
	// should be called at the beginning of each new line
	TextWriter& InsertTabs();

	UINT GetIndentation() const;
	TextWriter& IncreaseIndentation();
	TextWriter& DecreaseIndentation();

private:
	friend class shSetIndentation;

	int		m_scopeDepth;	// scope {} nest level
	int		m_indentation;	// indentation level
};

// scope helper
class TextScope
{
	TextWriter &	m_writer;
	const String	m_name;
	const bool	m_bSemicolon;

public:
	enum Flags
	{
		EndWithSemicolon = BIT(0)
	};
	TextScope( TextWriter& tw, UINT flags = 0 )
		: m_writer( tw ), m_bSemicolon(flags & EndWithSemicolon)
	{
		m_writer.EnterScope();
	}
	~TextScope()
	{
		m_writer.LeaveScope(m_bSemicolon);
	}
};

class shSetIndentation
{
	TextWriter &	m_writer;
	const UINT	m_oldIndentation;

public:
	shSetIndentation( TextWriter& tw, UINT newIndentation )
		: m_writer( tw ), m_oldIndentation( tw.m_indentation )
	{
		mxASSERT(newIndentation < MAX_INDENT_LEVEL);
		m_writer.m_indentation = newIndentation;
	}
	~shSetIndentation()
	{
		m_writer.m_indentation = m_oldIndentation;
	}
};

class shDecrementIndentation : public shSetIndentation
{
public:
	shDecrementIndentation( TextWriter& tw )
		: shSetIndentation( tw, tw.GetIndentation()-1 )
	{
	}
};
class shIncrementIndentation : public shSetIndentation
{
public:
	shIncrementIndentation( TextWriter& tw )
		: shSetIndentation( tw, tw.GetIndentation()+1 )
	{
	}
};

inline void WriteTextLabel( TextWriter& tw, const char* label )
{
	shDecrementIndentation di( tw );
	tw.PrintF( label );
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
