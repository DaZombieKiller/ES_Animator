// file: templ.h

#ifndef __TEMPL_H__INCLUDED__
#define __TEMPL_H__INCLUDED__

template<class _T>
class TPtrArray {
private:

	int countAllocated;
	int countUsed;
	_T* dataArray;

public:

	__inline TPtrArray( ){
		countAllocated = countUsed = 0;
		dataArray = 0;
	};

	__inline TPtrArray( int size ){
		countAllocated = countUsed = size;
		dataArray = (size > 0) ? (_T*)malloc( sizeof(_T) * size ) : 0;
	};

	__inline ~TPtrArray( ){
		if( countAllocated > 0 )
			free( dataArray );
	};

	__inline int size() {
		return countUsed; };

	__inline bool empty() {
		return (countUsed==0); };

	__inline void reserve( int count ){
		if( countAllocated < count ){
			if( countAllocated == 0 ){
				countAllocated = count;
				dataArray = (_T*)malloc( count * sizeof(_T) );
			} else {
				int nsize = count - countAllocated;
				int rlcsize = nsize + (countAllocated>>3);
				countAllocated += rlcsize;
				dataArray = (_T*)realloc( dataArray, countAllocated * sizeof(_T) );
			}
		} else if( count==0 ) {
			clear();
		}
	};

	__inline void resize( int count ){
		reserve( countUsed = count );
	};

	__inline void zero( int first, int last ){
		if( (last - first) > 0 ){
			_ASSERTE( first >= 0 && last <= countUsed );
			memset( dataArray + first, 0, (last - first) * sizeof(_T) );
		}
	};

	__inline void clear(){
		if( countAllocated > 0 ){
			_ASSERTE( dataArray );
			free( dataArray ); }
		countAllocated = countUsed = 0;
	};

	__inline void clear_nofree(){
		countUsed = 0;
	};

	__inline void erase( int index ){
		_ASSERTE( index >= 0 && index < countUsed );
		for(int i=index; i<(countUsed-1); ++i)
			memcpy( dataArray + i, dataArray + i + 1, sizeof(_T) );
		if( (--countUsed) == 0 )
			clear();
	};

	__inline void removeall( _T &_element ) {
/*		for(int i=0; i<countUsed;i++) 
			if( _element == dataArray[i] )
				erase( i-- );*/
	};

	__inline void remove( _T &_element ) {
		/*for(int i=0; i<countUsed;i++)
			if(_element ==  dataArray[i] ){
				erase( i );
				break;
			}*/
	};

	__inline _T& operator[] (int index) {
		_ASSERTE( index >= 0 && index < countUsed );
		return dataArray[index]; };

	__inline _T& front() {
		_ASSERTE( countUsed > 0 );
		return dataArray[0]; };

	__inline _T& back() {
		_ASSERTE( countUsed > 0 );
		return dataArray[countUsed-1]; };

	__inline _T* Ptr(){
		return dataArray; };

	__inline void swap( int index1, int index2 ){
		_ASSERTE( index1 >= 0 && index1 < countUsed );
		_ASSERTE( index2 >= 0 && index2 < countUsed );
		_T tmp;
		memcpy( &tmp, dataArray + index1, sizeof(_T) );
		memcpy( dataArray + index1, dataArray + index2, sizeof(_T) );
		memcpy( dataArray + index2, &tmp, sizeof(_T) );
	};

	__inline void push_back( _T& value ){
		reserve( countUsed + 1 );
		memcpy( dataArray + countUsed, & value,  sizeof(_T) );
		countUsed ++;
	};

	__inline void copy( TPtrArray<_T> & from ){
		if( from.size() > 0 ){
			reserve( countUsed = from.size() );
			memcpy( dataArray, from.Ptr(), countUsed * sizeof(_T) );
		} else {
			clear();
		}
	};

	__inline void copy( const TPtrArray<_T> & from ){
		if( from.countUsed > 0 ){
			reserve( countUsed = from.countUsed );
			memcpy( dataArray, from.dataArray, countUsed * sizeof(_T) );
		} else {
			clear();
		}
	};

	__inline void duplicate( DWORD index ){
		_ASSERTE( index >= 0 && index < countUsed );
		resize( countUsed + 1 );
		memcpy( dataArray + countUsed, dataArray + index,  sizeof(_T) );
		countUsed ++;
	};
};


#define SAFE_FREE( ptr )\
{\
	if( (ptr) ){\
		free( (ptr) );\
		(ptr) = 0;\
	}\
}
#define SAFE_DELETE( ptr )\
{\
	if( (ptr) ){\
		delete (ptr);\
		(ptr) = 0;\
	}\
}
#define SAFE_DELETE_A( ptr )\
{\
	if( (ptr) ){\
		delete [] (ptr);\
		(ptr) = 0;\
	}\
}

#endif
