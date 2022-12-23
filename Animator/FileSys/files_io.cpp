// file: Script.cpp

//#include "pch.h"
//#include "pch.h"
//#include "..\\FileSys\\templ.h"
//#include "..\\FileSys\\file_io.h"

CScript::CScript(){
	m_CharCount = 0;
	m_Buffer = 0;
	LineCount = 0;
}

CScript::CScript( char *source ){
	m_CharCount = 0;
	m_Buffer = 0;
	LineCount = 0;
	LoadString( source );
}

CScript::~CScript(){
	Clear();
}

bool CScript::LoadFromFile( char* name)
{
	DWORD d=0;
	HANDLE hlog = CreateFile(name, 
		               GENERIC_READ, 
					   FILE_SHARE_READ, NULL, 
					   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hlog== INVALID_HANDLE_VALUE) return false;
	long size = GetFileSize(hlog, NULL);

	SetFilePointer(hlog, 0,0,FILE_BEGIN);

	if( (m_CharCount = size) <= 0 )	{CloseHandle(hlog); return false;}

	m_Buffer = (char*)malloc( m_CharCount+1 );
	
	ReadFile( hlog, m_Buffer, m_CharCount, &l, NULL );
	if (l<m_CharCount) {CloseHandle(hlog); return false;}

	m_Buffer[m_CharCount] = 0;
	CloseHandle(hlog);
	return parse();
}

bool CScript::LoadString( char *source ){

	_ASSERTE( source );

	if( (m_CharCount = strlen( source )) <= 0 )
		return false;

	m_Buffer = (char*)malloc( m_CharCount+1 );
	memcpy( m_Buffer, source, m_CharCount+1 );

	return parse();
}

void CScript::Clear(){
	SAFE_FREE( m_Buffer );
	m_CharCount = 0;
	m_Tokens.clear();
}

long CScript::GetTokenCount(){
	return m_Tokens.size();
}

TOK *CScript::GetToken( long index ){
	if( index >= 0 && index < m_Tokens.size() )
		return (m_Tokens.Ptr() + index);
	return 0;
}

char *CScript::GetText(){
	return m_Buffer;
}

//**************************************************************************

bool CScript::parse()
{
	_ASSERTE( m_CharCount );
	_ASSERTE( m_Buffer );

	TOK tok;

	cc = 0;
	while( cc<m_CharCount ){

		skipspaces();

		tok.m_Type = TT_UNDEFINED;
		tok.m_BaseOffset = cc;
		tok.m_Size = 0;

		if( !trystring(tok) )
			if( !trysymbol(tok) )
				if( !trynumber(tok) ){
					tok.m_Type = TT_CHAR;
					tok.m_Char = m_Buffer[tok.m_BaseOffset];
					tok.m_Size ++;
					cc++;
				}

		tok.m_line = LineCount;
		m_Tokens.push_back( tok );
	}

	return true;
}

void CScript::skipcmt()
{
	while( (cc<m_CharCount) && !(
		m_Buffer[cc] == '\r' || 
		m_Buffer[cc] == '\n' ) ) cc++;
	while( (cc<m_CharCount) && (
		m_Buffer[cc] == ' ' || 
		m_Buffer[cc] == '\r' || m_Buffer[cc] == '\n' || 
		m_Buffer[cc] == '\t' ) ) { if (m_Buffer[cc] == '\n') LineCount++;
		cc++;}
}

void CScript::skipspaces()
{
	//int end_line_found = 0;
	while( (cc<m_CharCount) )
		if (m_Buffer[cc] == ' ') {cc++;} else
		if (m_Buffer[cc] == '\t' || m_Buffer[cc] == '\r') cc++; else
		if (m_Buffer[cc] == '\n') {cc++; LineCount++;} else
		break;

	if( (cc<(m_CharCount-2)) ){
		if( m_Buffer[cc] == '/' && m_Buffer[cc+1] == '/' ){
			skipcmt();
			skipspaces();
		}
	}
}

bool CScript::trystring(TOK& tok)
{
	if( m_Buffer[cc] == '\"' || m_Buffer[cc] == '\'' ){
		cc++;
		tok.m_BaseOffset ++;
		while( (cc<m_CharCount) && m_Buffer[cc] != '\"' && m_Buffer[cc] != '\'' ){
			tok.m_Size++;
			cc++;
		}
		cc++;
		tok.m_Type = TT_STRING;
		return true;
	}
	return false;
}

bool CScript::trysymbol(TOK& tok)
{
	if( isalpha(m_Buffer[cc]) || m_Buffer[cc] == '_' ){
		while( (cc<m_CharCount) && (
				m_Buffer[cc] == '_' || 
				isalpha(m_Buffer[cc]) || isdigit(m_Buffer[cc])) ){
			tok.m_Size++;
			cc++;
		}
		tok.m_Type = TT_SYMBOL;
		return true;
	}
	return false;
}

bool CScript::trynumber(TOK& tok)
{
	if( isdigit(m_Buffer[cc]) || m_Buffer[cc] == '+' || m_Buffer[cc] == '-'){
		bool bfloat = false;
		while( (cc<m_CharCount) && (
			m_Buffer[cc] == '.' || 
			isdigit(m_Buffer[cc]) ||
			m_Buffer[cc]=='+' || m_Buffer[cc]=='-' || 
			m_Buffer[cc]=='e' || m_Buffer[cc]=='E' ) ){
				if(m_Buffer[cc] == '.')
					bfloat = true;
				tok.m_Size++;
				cc++;
		}
		
		char tmp[64];
		memcpy(tmp,m_Buffer+tok.m_BaseOffset,tok.m_Size);
		tmp[tok.m_Size] = 0;

		if( bfloat ){
			tok.m_Type = TT_FLOAT;
			tok.m_Float = atof( tmp );
		} else {
			tok.m_Type = TT_INT;
			tok.m_Int  = atoi( tmp );
		}

		return true;
	}
	return false;
}

//**************************************************************************
// CBlock
//**************************************************************************


CBlock::CBlock(){
	Init();
}

CBlock::CBlock( CScript *p ){
	Init( p );
}

CBlock::CBlock( CBlock *p, long fst_q ){
	Init( p, fst_q );
}

CBlock::~CBlock(){
}

bool CBlock::Init(  ){
	m_Script = 0;
	m_Size = m_From = m_CP = 0;
	return true;
}

bool CBlock::Init( CScript *p ){
	if( (m_Script = p)==0 )
		return false;
	m_Size = p->GetTokenCount();
	m_From = 0;
	m_CP = 0;
	return true;
}

bool CBlock::Init( CBlock *p, long fst_q ){
	
	_ASSERTE( fst_q>=0 );
	_ASSERTE( p );

	m_Script = p->m_Script;
	m_From = p->m_From + fst_q;
	m_Size = 0;
	m_CP = 0;

	if( !parse() )
		return false;

	return true;
}

int CBlock::GetCurLine(){
	_ASSERTE( m_Script );
	TOK *T = m_Script->GetToken(m_CP+m_From);
	return T->m_line;
}

bool CBlock::GetInt( int *buffer ){
	_ASSERTE( buffer );
	_ASSERTE( m_Script );
	TOK *T = m_Script->GetToken(m_CP+m_From);
	if( T && T->m_Type==TT_INT ){
		(*buffer) = T->m_Int;
		return true;
	}
	return false;
}

bool CBlock::GetFloat( float *buffer ){
	_ASSERTE( buffer );
	_ASSERTE( m_Script );
	TOK *T = m_Script->GetToken(m_CP+m_From);
	if( T && T->m_Type==TT_FLOAT ){
		(*buffer) = T->m_Float;
		return true;
	}
	return false;
}

bool CBlock::GetChar( char *buffer ){
	_ASSERTE( buffer );
	_ASSERTE( m_Script );
	TOK *T = m_Script->GetToken(m_CP+m_From);
	if( T && T->m_Type==TT_CHAR ){
		(*buffer) = T->m_Char;
		return true;
	}
	return false;
}

bool CBlock::CmpChar( char to_compare ){
	_ASSERTE( m_Script );
	TOK *T = m_Script->GetToken(m_CP+m_From);
	if( T && T->m_Type==TT_CHAR ){
		if (T->m_Char == to_compare);
		return true;
	}
	return false;
}

bool CBlock::GetString( char *buffer ){
	_ASSERTE( buffer );
	_ASSERTE( m_Script );
	TOK *T = m_Script->GetToken(m_CP+m_From);
	if( T && T->m_Type==TT_STRING ){
		memcpy( buffer, m_Script->GetText()+T->m_BaseOffset,T->m_Size );
		buffer[T->m_Size] = 0;
		return true;
	}
	return false;
}

bool CBlock::GetSymbol( char *buffer ){
	_ASSERTE( buffer );
	_ASSERTE( m_Script );
	TOK *T = m_Script->GetToken(m_CP+m_From);
	if( T && T->m_Type==TT_SYMBOL ){
		memcpy( buffer, m_Script->GetText()+T->m_BaseOffset,T->m_Size );
		buffer[T->m_Size] = 0;
		return true;
	}
	return false;
}


bool CBlock::CmpSymbol( char *to_compare ){
	_ASSERTE( to_compare );
	_ASSERTE( m_Script );
	TOK *T = m_Script->GetToken(m_CP+m_From);
	if( T && T->m_Type==TT_SYMBOL ){
		char *_C = m_Script->GetText() + T->m_BaseOffset;
		for( int i=0; i<T->m_Size; i++, _C++ ){
			if( to_compare[i]==0 )
				return false;
			if( to_compare[i] != (*_C) )
				return false;
		}
		return (to_compare[i]==0);
	}
	return false;
}

bool CBlock::CmpString( char *to_compare ){
	_ASSERTE( to_compare );
	_ASSERTE( m_Script );
	TOK *T = m_Script->GetToken(m_CP+m_From);
	if( T && T->m_Type==TT_STRING ){
		char *_C = m_Script->GetText() + T->m_BaseOffset;
		for( int i=0; i<T->m_Size; i++, _C++ ){
			if( to_compare[i]==0 )
				return false;
			if( to_compare[i] != (*_C) )
				return false;
		}
		return (to_compare[i]==0);
	}
	return false;
}


void CBlock::Seek( long to ){
	m_CP = to;
	m_CP = max( m_CP, 0 );
	m_CP = min( m_CP, m_Size );
}

void CBlock::Skip( long amount ){
	Seek( m_CP + amount );
}

long CBlock::GetSize(){
	return m_Size;
}

long CBlock::GetCP(){
	return m_CP;
}

bool CBlock::IsEnd(){
	return (m_CP<0 || m_CP>=m_Size);
}


//**************************************************************************

bool CBlock::parse(){

	_ASSERTE( m_Script );

	TOK *T = m_Script->GetToken(m_From);
	if( !( T && T->m_Type==TT_CHAR && T->m_Char=='{' ) )
		return false;

	m_From++;
	int qcc = -1;

	while( (m_From+m_Size) < m_Script->GetTokenCount() ){
		TOK *T = m_Script->GetToken(m_From+m_Size);
		if( T && T->m_Type==TT_CHAR && T->m_Char=='{' ) qcc--;
		else if( T && T->m_Type==TT_CHAR && T->m_Char=='}' ) qcc++;
		if( qcc == 0 ) break;
		m_Size++;
	}

	if( (m_From+m_Size) >= m_Script->GetTokenCount() )
		return false;

	return true;
}


//**************************************************************************


//FILES WORK
long GetFileSize(char *str)
{
	HANDLE hlog = CreateFile(str, 
		               GENERIC_READ, 
					   FILE_SHARE_READ, NULL, 
					   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hlog== INVALID_HANDLE_VALUE) return -1;
	long size = GetFileSize(hlog, NULL);
    CloseHandle(hlog);
	return size;
}

/////////////////////////////////////////////////////////////////////////////