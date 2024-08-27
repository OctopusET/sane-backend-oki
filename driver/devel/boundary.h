/*
 * boundary.h
 * boundary objects.
 * Copyright (c) 2012 Oki Data Corporation. All rights reserved.
 */

#ifndef BOUNDARY_H_
#define BOUNDARY_H_
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <typeinfo>

#include "odc_define.h"

#if 0
#define __TRC(...) printf(__VA_ARGS__)
#else
#define __TRC(...)
#endif

namespace odc {


struct Storage
{
private:
	uint32_t _allocated : 1;
	uint32_t _size : 31;
	uint8_t* _storage;

	struct AllocatedStorage
	{
		uint32_t _reference;
		uint8_t _allocated_memory[0];
	};

public:
	Storage(size_t size);
	Storage(uint8_t* storage, size_t size);
	virtual ~Storage();
	size_t size() const {return this->_size;}
	uint8_t* storage() const;
	void shallow_copy(const Storage& o);
};



struct String : protected Storage
{
public:
	String(size_t size) : Storage(size+1) {}
	String(char* s, size_t size) : Storage(reinterpret_cast<uint8_t*>(s), size+1) {}
	String(char* s) : Storage(reinterpret_cast<uint8_t*>(s), strlen(s)+1) {}
	String(const char* s) : Storage(reinterpret_cast<uint8_t*>(const_cast<char*>(s)), strlen(s)+1) {}
	~String() {}

	size_t length() const {return strlen(reinterpret_cast<const char*>(this->storage()));}
	void set(const char* s, size_t len);
	void set(const char* s) {this->set(s, strlen(s));}
	const char* get() const {return reinterpret_cast<const char*>(this->storage());}
	bool equals(const char*s, size_t len) const;
	bool equals(const String& o) const {return this->equals(o.get(), o.length());}
	void copy(const char* s) {this->set(s);}

	bool operator == (const char* s) const {return this->equals(s, strlen(s));}
	bool operator == (const String& o) const {return this->equals(o);}
	bool operator != (const char* s) const {return !(this->equals(s, strlen(s)));}
	bool operator != (const String& o) const {return !(this->equals(o));}
	String& operator = (const String& o) {this->shallow_copy(o); return *this;}
	String& operator = (const char* s) {this->copy(s); return *this;}
};



template <class T>
struct XY
{
	T x;
	T y;

	XY()
	{
	}

	XY(const T x, const T y)
	{
		this->set(x, y);
	}

	XY& copy(const XY& o)
	{
		this->x = o.x;
		this->y = o.y;
		return *this;
	}

	XY& operator=(const XY& o)
	{
		return this->copy(o);
	}

	void set(const T x, const T y)
	{
		this->x = x;
		this->y = y;
	}
};


template <class T>
struct Mat
{
	XY<int> size;
	T* data;

	Mat(int n, T* storage) : size(n, n)
	{
		this->data = storage;
	}

	Mat(int x, int y, T* storage) : size(x, y)
	{
		this->data = storage;
	}

	T get(int x, int y) const
	{
		return this->data[x+y*size.x];
	}

	void set(int x, int y, T v)
	{
		this->data[x+y*size.x] = v;
	}

	int w() const
	{
		return this->size.x;
	}

	int h() const
	{
		return this->size.y;
	}
};




class Matcher
{
public:
	virtual bool match(const void* o) = 0;
	virtual ~Matcher() {}
};

#define ARYLST_INIT_SIZE (8)
#define ARYLST_GROW_SIZE (8)

template <class T>
class ArrayList
{
protected:
	size_t maxnum;				
	size_t num;					
	T* memp;						

public:
	ArrayList()
	{
		ArrayList<T>::init();
	}

	virtual ~ArrayList()
	{
		__TRC("ArrayList::~ArrayList()\n");
		ArrayList<T>::clean();
	}

	void init()
	{
		__TRC("ArrayList::init()\n");
		this->maxnum = 0;
		this->num = 0;
		this->memp = NULL;
	}

	void clean()
	{
		__TRC("ArrayList::clean()\n");
		if (this->memp != NULL)
		{
			free(this->memp);
		}
		init();
	}

	void add(T& v)
	{
		if (this->memp == NULL)
		{
			this->maxnum = ARYLST_INIT_SIZE;
			this->num = 0;
			size_t msz = sizeof(T) * (this->maxnum + 1);
			this->memp = static_cast<T*>(malloc(msz));
			memset(this->memp, 0, msz);
		}
		this->num ++;
		if (this->num > this->maxnum)
		{
			T* old = this->memp;
			this->maxnum += ARYLST_GROW_SIZE;
			size_t msz = sizeof(T) * (this->maxnum + 1);
			this->memp = static_cast<T*>(malloc(msz));
			memset(this->memp, 0, msz);
			for (size_t i=0 ; i<this->num-1 ; i++)
			{
				this->memp[i] = old[i];
			}
			free(old);
		}
		this->memp[this->num-1] = v;
	}

	void ins(unsigned i, T& v)
	{
		if (i >= this->num)
		{
			throw INVARG;
		}
		if (this->memp == NULL)
		{
			throw ILLSTATE;
		}
		this->num ++;
		if (this->num > this->maxnum)
		{
			T* old = this->memp;
			this->maxnum += ARYLST_GROW_SIZE;
			size_t msz = sizeof(T) * (this->maxnum + 1);
			this->memp = static_cast<T*>(malloc(msz));
			memset(this->memp, 0, msz);
			size_t j;
			for (j=0 ; j<i-1 ; j++)
			{
				this->memp[j] = old[j];
			}
			this->memp[j] = v;
			for (j++ ; j<this->num-1 ; j++)
			{
				this->memp[j] = old[j-1];
			}
			free(old);
		}
		else
		{
			size_t j;
			for (j=this->num-2 ; j>=i ; j--)
			{
				this->memp[j+1] = this->memp[j];
			}
			this->memp[i] = v;
		}
	}

	void del(unsigned i)
	{
		if (i < this->num)
		{
			unsigned j;
			for (j=i+1 ; j<this->num ; j++)
			{
				this->memp[j-1] = this->memp[j];
			}
			this->num --;
		}
	}

	T& get(unsigned i) const
	{
		if (i<0 || this->num<=i)
		{
#ifdef ENABLE_DBGTOOL
			fprintf(stderr, "array index out of range. index=%d (range:0...%d)\n", i, this->num-1);
#endif
			throw ARRAY_INDEX_OUT_OF_RANGE;
		}
		return this->memp[i];
	}

	T& operator[] (unsigned i) const
	{
		return this->get(i);
	}


	int find(Matcher& m) const
	{
		unsigned i;
		for (i=0 ; i<this->size() ; i++)
		{
			if (m.match(this->memp[i]))
			{
				return i;
			}
		}
		return -1;
	}

	int rfind(Matcher& m) const
	{
		int i;
		for (i=this->size()-1 ; i>=0 ; i--)
		{
			if (m.match(this->memp[i]))
			{
				return i;
			}
		}
		return -1;
	}

	int find(T& o) const
	{
		unsigned i;
		for (i=0 ; i<this->size() ; i++)
		{
			if (o == this->memp[i])
			{
				return i;
			}
		}
		return -1;
	}

	int rfind(T& o) const
	{
		int i;
		for (i=this->size()-1 ; i>=0 ; i--)
		{
			if (o == this->memp[i])
			{
				return i;
			}
		}
		return -1;
	}


	size_t max() const
	{
		return this->maxnum;
	}

	size_t size() const
	{
		if (this->memp == NULL)
			return 0;
		return this->num;
	}

	T* toArrayPtr() const
	{
		return this->memp;
	}
};


template <class T> class ValueList : public ArrayList<T>
{
protected:
	virtual T _parseval(const char* sp, char** np) = 0;

public:
	virtual ~ValueList() {}

	int parse(const char*s)
	{
		const char* sp = s;
		char* np;
		T v;
		this->clean();
		while (true) {
			v = _parseval(sp, &np);
			if (sp == np) {
				break;
			}
			if (!(*np == ',' || *np == '\0' || *np == ' ' || *np == '\t')) {
				return -1;
			}
			this->add(v);
			while (*np == ',' || *np == ' ' || *np == '\t') np ++;
			sp = np;
		}
		return this->size();
	}

};

class IntValueList : public ValueList<int>
{
protected:
	virtual int _parseval(const char* sp, char** np) {return strtol(sp, np, 0);}
};

class FloatValueList : public ValueList<double>
{
protected:
	virtual double _parseval(const char* sp, char** np) {return strtof(sp, np);}
};



class StringList : public ArrayList<String>
{

};



template <class T>
class PointerList : public ArrayList<T*>
{
protected:
	bool clean_with_delete;		

public:
	PointerList(bool clean_with_delete=false)
	{
		init(clean_with_delete);
	}

	virtual ~PointerList()
	{
		__TRC("PointerList::~PointerList()\n");
		clean();
	}

	void init(bool clean_with_delete=false)
	{
		__TRC("PointerList::init(%d)\n",clean_with_delete);
		ArrayList<T*>::init();
		this->clean_with_delete=clean_with_delete;
	}

	void clean()
	{
		__TRC("PointerList::clean()\n");
		if (this->clean_with_delete)
		{
			for (size_t i=0 ; i<this->size() ; i++)
			{
				__TRC("delete %p\n", this->memp[i]);
				const std::type_info& id = typeid(this->memp[i]);
				const char* n = id.name();
				if (n[0]=='P' && n[1]=='c') {
					free(this->memp[i]);
				} else {
					delete this->memp[i];
				}
			}
		}
		ArrayList<T*>::clean();
	}

	void del(unsigned i)
	{
		if (i < this->num)
		{
			if (this->clean_with_delete)
			{
				const std::type_info& id = typeid(this->memp[i]);
				const char* n = id.name();
				if (n[0]=='P' && n[1]=='c') {
					free(this->memp[i]);
				} else {
					delete this->memp[i];
				}
			}

			unsigned j;
			for (j=i+1 ; j<this->num ; j++)
			{
				this->memp[j-1] = this->memp[j];
			}
			this->num --;
		}
	}

	T* operator[] (unsigned i) const
	{
		return this->get(i);
	}
};


struct Property
{
	enum Type
	{
		STRING,
		INTEGER,
		FLOAT,
		CHAR,
		BOOL
	};

protected:
	char name[MAX_PROPNAME_LEN];

public:
	Property();
	Property(const char* name);
	virtual ~Property() {}
	bool is_name(const char* name) const {return (name != NULL && strcmp(this->name, name) == 0);}
	const char* get_name() const {return this->name;}

	virtual Type type() const = 0;
	virtual void copy(const Property& o) = 0;
	virtual const void* get_value() const = 0;
	virtual size_t get_value_size() const = 0;
	virtual bool equals(const Property& o) const = 0;
	virtual Property* clone() const = 0;
	virtual int _str(char* buf) const = 0;

	Property& operator = (const Property& o) {this->copy(o); return *this;}
	bool operator == (const Property& o) const {return equals(o);}
};

struct StringProperty : public Property
{
protected:
	char value[MAX_PORPSTR_LEN];

public:
	StringProperty();
	StringProperty(const char* name);
	StringProperty(const char* name, const char* value);
	StringProperty(const StringProperty& o) : Property() {this->copy(o);}
	void set_value(const char* value);
	virtual Type type() const;
	virtual void copy(const Property& o);
	virtual const void* get_value() const {return this->value;}
	virtual size_t get_value_size() const {return strlen(this->value);}
	virtual bool equals(const Property& o) const;
	virtual Property* clone() const {return new StringProperty(*this);}
	virtual int _str(char* buf) const;
};

struct IntegerProperty : public Property
{
protected:
	int value;

public:
	IntegerProperty();
	IntegerProperty(const char* name);
	IntegerProperty(const char* name, int value);
	IntegerProperty(const IntegerProperty& o) : Property() {this->copy(o);}
	void set_value(int value);
	virtual Type type() const;
	virtual void copy(const Property& o);
	virtual const void* get_value() const {return &this->value;}
	virtual size_t get_value_size() const {return sizeof(int);}
	virtual bool equals(const Property& o) const;
	virtual Property* clone() const {return new IntegerProperty(*this);}
	virtual int _str(char* buf) const;
};

struct FloatProperty : public Property
{
protected:
	double value;

public:
	FloatProperty();
	FloatProperty(const char* name);
	FloatProperty(const char* name, double value);
	FloatProperty(const FloatProperty& o) : Property() {this->copy(o);}
	void set_value(double value);
	virtual Type type() const;
	virtual void copy(const Property& o);
	virtual const void* get_value() const {return &this->value;}
	virtual size_t get_value_size() const {return sizeof(double);}
	virtual bool equals(const Property& o) const;
	virtual Property* clone() const {return new FloatProperty(*this);}
	virtual int _str(char* buf) const;
};

struct CharProperty : public Property
{
protected:
	unsigned char value[MAX_PORPSTR_LEN-sizeof(size_t)];
	size_t valid_len;

public:
	CharProperty();
	CharProperty(const char* name);
	CharProperty(const char* name, const unsigned char* value, size_t value_len);
	CharProperty(const char* name, const unsigned char* string_value);
	CharProperty(const CharProperty& o) : Property() {this->copy(o);}
	void set_value(const unsigned char* value, size_t value_len);
	void set_value(const unsigned char* string_value);
	void to_str(char* s) const;
	virtual Type type() const;
	virtual void copy(const Property& o);
	virtual const void* get_value() const {return this->value;}
	virtual size_t get_value_size() const {return this->valid_len;}
	virtual bool equals(const Property& o) const;
	virtual Property* clone() const {return new CharProperty(*this);}
	virtual int _str(char* buf) const;
};

struct BoolProperty : public Property
{
protected:
	bool value;

public:
	BoolProperty();
	BoolProperty(const char* name);
	BoolProperty(const char* name, bool value);
	BoolProperty(const BoolProperty& o) : Property() {this->copy(o);}
	void set_value(bool value);
	virtual Type type() const;
	virtual void copy(const Property& o);
	virtual const void* get_value() const {return &this->value;}
	virtual size_t get_value_size() const {return sizeof(bool);}
	virtual bool equals(const Property& o) const;
	virtual Property* clone() const {return new BoolProperty(*this);}
	virtual int _str(char* buf) const;
};





struct Properties : public odc::PointerList<Property>
{
	enum DupAct {ALLOW, DENY, OVERWRITE};

	Properties();
	int add(Property* p, DupAct act=DENY);
	const void* get_value(const char* name) const;
	int findi(const char* name, unsigned stidx = 0) const;
	const Property* find(const char* name) const;
	const char* find_str(const char* name) const;
	const int* find_int(const char* name) const;
	const double* find_float(const char* name) const;
	const unsigned char* find_byte(const char* name) const;
	const bool* find_bool(const char* name) const;
	bool has(const Property& p) const;
	int _str(char* buf) const;
};




struct PropertiesHolder : public odc::PointerList<odc::Properties>
{
	PropertiesHolder();
	int findi(const Property& key, unsigned stidx = 0) const;
	const Properties* find(const Property& key) const;
	void _trace(int level) const;
};






class Chain
{
protected:
	Chain* _next;	
	Chain* _prev;	

public:
	Chain();
	virtual ~Chain(){};
	bool isalone() const {return (this->_next==this->_prev && this->_next==this);}
	Chain* next() const {return this->_next;}
	Chain* prev() const {return this->_prev;}
	void addnext(Chain* p);
	void addprev(Chain* p);
	void remove();
};


}; 

#endif /* BOUNDARY_H_ */
