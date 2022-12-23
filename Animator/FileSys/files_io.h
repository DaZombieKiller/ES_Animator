// file: Script.h
// low-level script parser

#ifndef __SCRIPT_H__INCLUDED__
#define __SCRIPT_H__INCLUDED__

#pragma warning( disable:4251 )

typedef enum {

	TT_UNDEFINED	= 0,
	TT_STRING		= 1,
	TT_SYMBOL		= 2,
	TT_INT			= 3,
	TT_FLOAT		= 4,
	TT_CHAR			= 5,

	TT_FORCE_DWORD	= 0xffffffff

} TOKENTYPE;


#pragma pack(push,1)

typedef struct TOK {

	TOKENTYPE m_Type;

	long m_line;
	long m_BaseOffset;
	long m_Size;

	union {
		struct {
			long m_Int;
		};
		struct {
			float m_Float;
		};
		struct {
			char m_Char;
		};
	};
};

#pragma pack(pop)


class CScript
{
private:
	// original text
	long m_CharCount;
	char *m_Buffer;

	// tokens
	TPtrArray<TOK> m_Tokens;

public:

	CScript();
	//bi_BScript( bi_DataStream& source );
	CScript( char *source );
	~CScript();

	bool LoadFromFile( char *name );
	bool LoadString( char *source );
	void Clear();

	long GetTokenCount();
	TOK *GetToken( long index );
	char *GetText();

private:
	long LineCount;
	int cc;
	
	bool parse();
	void skipcmt();
	void skipspaces();
	bool trystring(TOK& tok);
	bool trysymbol(TOK& tok);
	bool trynumber(TOK& tok);
};




class CBlock
{
private:

	CScript *m_Script;	// parent script
	long m_Size;			// N of tokens included (may be 0)
	long m_From;			// fst token after '{'
	long m_CP;				// current read position

public:

	CBlock();
	CBlock( CScript *p );
	CBlock( CBlock *p, long fst_q );
	~CBlock();

	bool Init( );						// empty (for internal use)
	bool Init( CScript *p );			// from script
	bool Init( CBlock *p, long fst_q );	// fst_q is index of '{'...

	int  GetCurLine();
	bool GetInt( int *buffer );
	bool GetFloat( float *buffer );
	bool GetChar( char *buffer );
	bool GetString( char *buffer );
	bool GetSymbol( char *buffer );

	bool CmpChar( char to_compare );
	bool CmpSymbol( char *to_compare );
	bool CmpString( char *to_compare );

	void Seek( long to );
	void Skip( long amount );
	long GetSize();
	long GetCP();
	bool IsEnd();

private:

	bool parse();
};



long GetFileSize(char *str);

#endif

