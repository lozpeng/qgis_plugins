
#ifndef XFILEIO_INCLUDE
#define XFILEIO_INCLUDE

#include <vector>
using namespace std;

typedef unsigned long       DWORD;

/************************************************************************/
/*           这是一个虚基类                                    */
/************************************************************************/
class CXFileIO
{
public:
	CXFileIO()
	{
		//最大值4MB
		MAXBUFMEMSIZE = 1 << 23;
	}
#ifdef SEEK_SET
	typedef enum {
		XFILEIO_SEEK_SET = SEEK_SET,
		XFILEIO_SEEK_CUR  =SEEK_CUR,
		XFILEIO_SEEK_END  =SEEK_END
	} CXFileIOSeek;
#else
	typedef enum {
		XFILEIO_SEEK_SET = 1,
		XFILEIO_SEEK_CUR  =2,
		XFILEIO_SEEK_END  =3
	} CXFileIOSeek;
#endif
	//获取当前位置
	virtual long tell() = 0;
	//判断是否出错
	virtual bool error() = 0;
	//定位
	virtual long seek(long offset, CXFileIOSeek seek) = 0;
	//读取
	virtual int  read(void * pbuf, long size) = 0;
	//写入
	virtual int  write(void *pbuf, long size) = 0;
	//总长度
	virtual 	long length() = 0;



	bool readString(wstring &s)
	{
		//读入一个dword长度
		DWORD cnt;
		if(read(&cnt, sizeof(DWORD)) != sizeof(DWORD))
			return false;

		//读入数据
		s= L"";
		for(DWORD i = 0; i < cnt; i++)
		{
			unsigned short c;
			if(read(&c, sizeof(unsigned short)) != sizeof(unsigned short))
				return false;
			s.push_back(c);
		}
		return true;
	}
	bool writeString(wstring &s)
	{
		//读入一个dword长度
		DWORD cnt = s.size();
		if(write(&cnt, sizeof(DWORD)) != sizeof(DWORD))
			return false;

		//读入数据
		for(DWORD i = 0; i < cnt; i++)
		{
			unsigned short c = s[i];
			if(write(&c, sizeof(unsigned short)) != sizeof(unsigned short))
				return false;
		}
		return true;
	}

	long  MAXBUFMEMSIZE;
};

/************************************************************************/
/*             基本的文件读取方式                                    */
/************************************************************************/
#ifdef USEIODEFAULT
class CXFileIODefault: public CXFileIO
{
public:
	CXFileIODefault()
	{
		m_pFile = NULL;
	}
	~CXFileIODefault()
	{
		if (m_pFile)
		{
			fclose(m_pFile);
		}
	}
	
	virtual long length(){
		if (!m_pFile)
			return 0;
		long cur = ftell(m_pFile);
		fseek(m_pFile,0, SEEK_END);

		long len = ftell(m_pFile);

		fseek(m_pFile, cur, SEEK_SET);

		return len;
		
	}


	//获取当前位置

	long tell()
	{
		if(!m_pFile)
			return 0;
		return ftell(m_pFile);
	}
	//判断是否出错
	bool error()
	{
		return false;
	}
	//定位
	long seek(long offset, CXFileIOSeek seek)
	{
		if(!m_pFile)
			return 0;
		return fseek(m_pFile,offset, seek);
	}
	//读取
	int  read(void * pbuf, long size)
	{
		if(!m_pFile)
			return 0;
		return fread(pbuf, 1, size, m_pFile);
	}
	//写入
	int  write(void *pbuf, long size)
	{
		if(!m_pFile)
			return 0;
		return fwrite(pbuf, 1, size, m_pFile);
	}

	bool   open(const char * pStr, char * flag)
	{
		return fopen_s(&m_pFile, pStr, flag) == 0;
	}
	void   close()
	{

		if (m_pFile)
		{
			fclose(m_pFile);
		}
		return;
	}

protected:
	FILE * m_pFile;
};
#endif
/************************************************************************/
/* 内存文件                                                                     */
/************************************************************************/
#ifndef ULONG
#define  ULONG unsigned long
#endif
#ifndef UINT
#define  UINT unsigned int
#endif

class CXFileIOMem:public CXFileIO
{
public:
	CXFileIOMem(){
		m_curPos =0;
	}
	virtual bool  error()
	{
		return false;
	}
	virtual long  tell()
	{
		return m_curPos;
	}
	virtual long  seek(long offset, CXFileIOSeek seek)
	{
		if (m_buf.empty())
		{
			return 0;
		}
		if (seek == CXFileIO::XFILEIO_SEEK_CUR)
		{
			offset+= m_curPos;
		}
		else if (seek == CXFileIO::XFILEIO_SEEK_END)
		{
			offset = m_buf.size() - offset;
		}
		if (offset >= 0 && offset < (long)m_buf.size())
		{
			m_curPos = offset;
		}
		return m_curPos;
	}
	virtual int  read(void *buf, long size)
	{
		if(buf == NULL)
			return 0;
		if (m_buf.size() <= m_curPos)
			return 0;

		
		long leftSize = m_buf.size() - m_curPos;
		
		//正常读出
		if (leftSize > size)
		{
			memcpy(buf, &m_buf[m_curPos],size);
			m_curPos += size;
			return size;
		}
		//字节不够，那么只读取剩余部分
		else
		{
			memcpy(buf, &m_buf[m_curPos],leftSize);
			m_curPos += leftSize;
			return leftSize;
		}
		return 0;
	}
	virtual int  write(void *buf, long size)
	{
	
		if(buf == NULL || size <= 0)
			return 0;

	    if(m_curPos > m_buf.size())
		{
			return 0;
		}
		if(m_curPos + size >  m_buf.size())
		{
			long srcsz = m_buf.size();
			m_buf.resize(srcsz + size);
			memcpy(&m_buf[srcsz] ,buf, size);
			m_curPos = m_buf.size();		
		}
		else
		{
			memcpy(&m_buf[m_curPos] ,buf, size);
			m_curPos += size;		
		}

		return size;
	}
	
	void putData(const void *pData, long size)
	{
		if (pData==NULL)
		{
			return;
		}
		if (size<=0)
		{
			return;
		}

		m_curPos = 0;
		m_buf.resize(size);
		memcpy(&m_buf[0], pData, size);
		
	}

	virtual long length(){
		return m_buf.size();
	}
	void * buffer(){
		return &m_buf[0];
	}

	void clear(){
		m_buf.clear();
		m_curPos = 0;
	}
	void resize(long sz){
		m_buf.clear();
		m_buf.resize(sz);
		m_curPos = 0;
	}

#ifdef USEIODEFAULT
	bool load(const char * strf){
		CXFileIODefault f;
		if (!f.open(strf,"rb"))
		{
			return false;
		}
		long sz = f.length();
		m_buf.resize(sz);
		f.read(&m_buf[0], sz);
		return true;
	}
#endif
protected:
private:
	vector<unsigned char> m_buf;
	unsigned long        m_curPos;
};

/************************************************************************/
/*                  stream流                       */
/************************************************************************/

#ifdef USEIOSTREAM
class CXFileIOStream:public CXFileIO
{
public:
	CXFileIOStream(){
		
	}
	virtual bool  error()
	{
		return false;
	}
	virtual long  tell()
	{
		if (!m_pStream)return 0;

		ULARGE_INTEGER pos;
		LARGE_INTEGER soff;
		soff.HighPart = 0;
		soff.LowPart = 0;
		m_pStream->Seek(soff, STREAM_SEEK_CUR, &pos);

		return pos.LowPart;
	}
	virtual long  seek(long offset, CXFileIOSeek seek)
	{
		if (!m_pStream)return 0;
		
		ULARGE_INTEGER pos;
		LARGE_INTEGER soff;
		soff.HighPart = 0;
		soff.LowPart = offset;
		switch (seek)
		{
		case XFILEIO_SEEK_CUR:
			m_pStream->Seek(soff, STREAM_SEEK_CUR, &pos);
			break;
		case XFILEIO_SEEK_SET:
			m_pStream->Seek(soff, STREAM_SEEK_SET, &pos);
			break;
		case XFILEIO_SEEK_END:
			m_pStream->Seek(soff, STREAM_SEEK_END, &pos);
			break;
		}
		return pos.LowPart;
	}
	virtual int  read(void *buf, long size)
	{
		if (!m_pStream)return 0;


		DWORD sz = 0;
		m_pStream->Read(buf, size, &sz);
		return sz;

	}
	virtual int  write(void *buf, long size)
	{
		if (!m_pStream)return 0;

		DWORD sz = 0;
		m_pStream->Write(buf, size, &sz);
		return sz;
	}


	virtual long length(){
		
		if (!m_pStream)return 0;
		//保存现在位置
		long pos = tell();

		LARGE_INTEGER l;
		l.HighPart = 0;
		l.LowPart = 0;
		ULARGE_INTEGER p0, p1;
		m_pStream->Seek(l, STREAM_SEEK_SET, &p0);
		m_pStream->Seek(l, STREAM_SEEK_END, &p1);

		//恢复
		l.LowPart = pos;
		m_pStream->Seek(l, STREAM_SEEK_SET,NULL);
		
		return p1.LowPart - p0.LowPart;
	}

	bool  Attach(IStream * pStream)
	{
		if (!pStream)
		{
			return false;
		}
		if (m_pStream)
		{
			m_pStream.Release();
		}
	
		pStream->QueryInterface(IID_IStream, (void **)&m_pStream);

		MAXBUFMEMSIZE = length();
		return true;
	}

protected:
private:
	CComPtr<IStream > m_pStream;
};
#endif //USEIOSTREAM

#define LOADVECTORMAXBUF  0xffffffff

#define  RETURNFALSE2(a) if(!(a)) return false;

#define  SAVEVECTOR(a, t) {\
	DWORD sz = a.size();\
	RETURNFALSE2(io.write(&sz, sizeof(DWORD)) == sizeof(DWORD));\
	if(sz > 0){\
	RETURNFALSE2(io.write(&a[0], sizeof(t) * sz) ==  sizeof(t) * sz);}\
}

#define  LOADVECTOR(a, t) {\
	DWORD sz;\
	RETURNFALSE2(io.read(&sz, sizeof(DWORD)) == sizeof(DWORD));\
	if(sz > 0 && sz < (DWORD)io.MAXBUFMEMSIZE){\
	a.resize(sz);\
	RETURNFALSE2(io.read(&a[0], sizeof(t) * sz) ==  sizeof(t) * sz);}\
}

#define SAVE(a) RETURNFALSE2(io.write(&a, sizeof(a)) == sizeof(a));

#define LOAD(a) RETURNFALSE2(io.read(&a, sizeof(a)) == sizeof(a));




//定义两个基本变量
#define DEFINEVAR   int sz =0; int srcPos =io.tell();
//返回失败的时候，要把io游标恢复
#define RETURNFALSE {io.seek(srcPos, CXFileIO::XFILEIO_SEEK_SET);return false;}
//读取
#define READ(a, size) {          \
	sz = io.read((a), (size));   \
	if(sz != (size))             \
	RETURNFALSE				 \
}
//写入
#define WRITE(a, size)  {		 \
	sz = io.write((a), (size));  \
	if(sz != (size))             \
	RETURNFALSE				 \
}

#define LOADEFUNC_BEGIN     bool  load(CXFileIO &io){DEFINEVAR
#define SAVEEFUNC_BEGIN     bool  save(CXFileIO &io){DEFINEVAR
#define FUNC_END            return true; }

//计算结构数据大小
#define CALCULSIZE_BEGIN    DWORD datasize;bool CaculSize(){datasize=0;

#define CALCULSIZE  CaculSize();

#endif