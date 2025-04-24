/*
 * odc_boudary.cpp
 *
 * Copyright (c) 2012 Oki Data Corporation. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "odc_boundary.h"

#include "odc_tracer.h"
#define TRCID "boundary"
#define TRACE(LEVEL, ERRNO, ...) T_TRACE((LEVEL), TRCID, ERRNO, __VA_ARGS__)
#define FATAL(ERRNO, ...) T_FATAL(TRCID, ERRNO, __VA_ARGS__)
#define ERR(ERRNO, ...) T_ERR(TRCID, ERRNO, __VA_ARGS__)
#define WRN(ERRNO, ...) T_WARN(TRCID, ERRNO, __VA_ARGS__)
#define INFOh(...) T_INFO_H(TRCID, __VA_ARGS__)
#define INFO(...) T_INFO(TRCID, __VA_ARGS__)
#define INFOl(...) T_INFO_L(TRCID, __VA_ARGS__)
#define MESSAGE(...) T_MESSAGE(TRCID, __VA_ARGS__)
#define DBG(...) T_DBG(TRCID, __VA_ARGS__)
#define TRC(...) T_TRC1(TRCID, __VA_ARGS__)
#define TRC2(...) T_TRC2(TRCID, __VA_ARGS__)
#define TRC3(...) T_TRC3(TRCID, __VA_ARGS__)
#define TRC4(...) T_TRC4(TRCID, __VA_ARGS__)
#define TMP(...) T_TMP(TRCID, 0, __VA_ARGS__)
#define DUMP(DATA, SIZE) T_DUMP(TRCID, DATA, SIZE)

namespace odc {

Storage::Storage(size_t size)
{
	AllocatedStorage* as;
	size_t off = offsetof(AllocatedStorage, _allocated_memory);
	//TMP("offsetof=%d", off);
	this->_size = size;
	this->_storage = static_cast<uint8_t*>(malloc(size + off));
	this->_allocated = true;
	as = reinterpret_cast<AllocatedStorage*>(this->_storage);
	memset(as->_allocated_memory, 0, this->_size);
	as->_reference = 1;
}
Storage::Storage(uint8_t* storage, size_t size)
{
	this->_size = size;
	this->_storage = storage;
	this->_allocated = false;
}
Storage::~Storage()
{
	if (this->_allocated) {
		AllocatedStorage* as = reinterpret_cast<AllocatedStorage*>(this->_storage);
		as->_reference --;
		if (as->_reference == 0) {
			free(this->_storage);
		}
	}
}

uint8_t* Storage::storage() const
{
	if (this->_allocated) {
		AllocatedStorage* as = reinterpret_cast<AllocatedStorage*>(this->_storage);
		return as->_allocated_memory;
	} else {
		return this->_storage;
	}
}

void Storage::shallow_copy(const Storage& o)
{
	this->_allocated = o._allocated;
	this->_size = o._size;
	this->_storage = o._storage;
	if (this->_allocated) {
		AllocatedStorage* as = reinterpret_cast<AllocatedStorage*>(this->_storage);
		as->_reference ++;
	}
}






void String::set(const char* s, size_t len)
{
	size_t l = len;
	if (l >= this->size()) {
		l = this->size() - 1;
	}
	memcpy(this->storage(), s, l);
	this->storage()[l] = '\0';
}

bool String::equals(const char*s, size_t len) const
{
	if (this->length() != len) {
		return false;
	}

	if (memcmp(this->storage(), s, len) != 0) {
		return false;
	}

	return true;
}





Property::Property()
{
	memset(this->name, 0, sizeof(this->name));
}
Property::Property(const char* name)
{
	strncpy(this->name, name, sizeof(this->name));
}







StringProperty::StringProperty() : Property()
{
}
StringProperty::StringProperty(const char* name) : Property(name)
{
}
StringProperty::StringProperty(const char* name, const char* value) : Property(name)
{
	this->set_value(value);
}
void StringProperty::set_value(const char* value)
{
	memset(this->value, 0, sizeof(this->value));
	strncpy(this->value, value, sizeof(this->value)-1);
}
Property::Type StringProperty::type() const
{
	return STRING;
}
void StringProperty::copy(const Property& o)
{
	const StringProperty* p = dynamic_cast<const StringProperty*>(&o);
	if (p == NULL) {
		throw INVARG;
	}
	memcpy(this->name, p->name, sizeof(this->name));
	memcpy(this->value, p->value, sizeof(this->value));
}
bool StringProperty::equals(const Property& o) const
{
	const StringProperty* p = dynamic_cast<const StringProperty*>(&o);
	if (p != NULL) {
		if (strcmp(this->name, p->name)==0) {
			if (strcmp(this->value, p->value)==0) {
				return true;
			}
		}
	}
	return false;
}
int StringProperty::_str(char* buf) const//for debug
{
	return sprintf(buf, "(string)'%s'=>'%s'", this->name, this->value);
}






IntegerProperty::IntegerProperty() : Property()
{

}
IntegerProperty::IntegerProperty(const char* name) : Property(name)
{

}
IntegerProperty::IntegerProperty(const char* name, int value) : Property(name)
{
	this->set_value(value);
}
void IntegerProperty::set_value(int value)
{
	this->value = value;
}
Property::Type IntegerProperty::type() const
{
	return INTEGER;
}
void IntegerProperty::copy(const Property& o)
{
	const IntegerProperty* p = dynamic_cast<const IntegerProperty*>(&o);
	if (p == NULL) {
		throw INVARG;
	}
	memcpy(this->name, p->name, sizeof(this->name));
	this->value = p->value;
}
bool IntegerProperty::equals(const Property& o) const
{
	const IntegerProperty* p = dynamic_cast<const IntegerProperty*>(&o);
	if (p != NULL) {
		if (strcmp(this->name, p->name)==0) {
			if (this->value == p->value) {
				return true;
			}
		}
	}
	return false;
}
int IntegerProperty::_str(char* buf) const//for debug
{
	return sprintf(buf, "(integer)'%s'=>%i(0x%08x)", this->name, this->value, this->value);
}





FloatProperty::FloatProperty() : Property()
{

}
FloatProperty::FloatProperty(const char* name) : Property(name)
{

}
FloatProperty::FloatProperty(const char* name, double value) : Property(name)
{
	this->set_value(value);
}
void FloatProperty::set_value(double value)
{
	this->value = value;
}
Property::Type FloatProperty::type() const
{
	return FLOAT;
}
void FloatProperty::copy(const Property& o)
{
	const FloatProperty* p = dynamic_cast<const FloatProperty*>(&o);
	if (p == NULL) {
		throw INVARG;
	}
	memcpy(this->name, p->name, sizeof(this->name));
	this->value = p->value;
}
bool FloatProperty::equals(const Property& o) const
{
	const FloatProperty* p = dynamic_cast<const FloatProperty*>(&o);
	if (p != NULL) {
		if (strcmp(this->name, p->name)==0) {
			if (this->value == p->value) {
				return true;
			}
		}
	}
	return false;
}
int FloatProperty::_str(char* buf) const//for debug
{
	return sprintf(buf, "(float)'%s'=>%f", this->name, this->value);
}





CharProperty::CharProperty() : Property()
{
	this->valid_len = 0;
}
CharProperty::CharProperty(const char* name) : Property(name)
{
	this->valid_len = 0;
}
CharProperty::CharProperty(const char* name, const unsigned char* value, size_t value_len) : Property(name)
{
	this->set_value(value, value_len);
}
CharProperty::CharProperty(const char* name, const unsigned char* string_value) : Property(name)
{
	this->set_value(string_value);
}
void CharProperty::set_value(const unsigned char* value, size_t value_len)
{
	this->valid_len = value_len;
	if (this->valid_len > sizeof(this->value)) {
		this->valid_len = sizeof(this->value);
	}
	memcpy(this->value, value, this->valid_len);
}
/**
 * 各バイト値をコロン区切りで表した文字列表現の値から設定を行う。
 * @param[in] string_value 文字列形式のCHAR型値（':'区切りの表記）
 */
void CharProperty::set_value(const unsigned char* string_value)
{
	int j;
	int cnt = 1;
	for (j=0 ; string_value[j]!='\0' ; j++) {
		if (string_value[j] == ':')
			cnt++;
	}
	unsigned char v[cnt];
	char* p = reinterpret_cast<char*>(const_cast<unsigned char*>(string_value));
	char* ep;
	for (j=0 ; j<cnt ; j++) {
		v[j] = strtol(p, &ep, 16);
		p = ep + 1;
	}
	set_value(v, cnt);
}
void CharProperty::to_str(char* s) const
{
	int off = 0;
	for (int i=0 ; i<this->valid_len ; i++) {
		if (i>0) {off += sprintf(&s[off], ":");}
		off += sprintf(&s[off], "%02x", this->value[i]);
	}
}
Property::Type CharProperty::type() const
{
	return CHAR;
}
void CharProperty::copy(const Property& o)
{
	const CharProperty* p = dynamic_cast<const CharProperty*>(&o);
	if (p == NULL) {
		throw INVARG;
	}
	memcpy(this->name, p->name, sizeof(this->name));
	memcpy(this->value, p->value, sizeof(this->value));
	this->valid_len = p->valid_len;
}
bool CharProperty::equals(const Property& o) const
{
	const CharProperty* p = dynamic_cast<const CharProperty*>(&o);
	if (p != NULL) {
		if (strcmp(this->name, p->name)==0) {
			if (this->valid_len == p->valid_len && memcmp(this->value, p->value, this->valid_len)==0) {
				return true;
			}
		}
	}
	return false;
}
int CharProperty::_str(char* buf) const//for debug
{
	int off = 0;
	off = sprintf(buf, "(char)'%s'=>", this->name);
	this->to_str(&buf[off]);
	return off;
}





BoolProperty::BoolProperty() : Property()
{

}
BoolProperty::BoolProperty(const char* name) : Property(name)
{

}
BoolProperty::BoolProperty(const char* name, bool value) : Property(name)
{
	this->set_value(value);
}
void BoolProperty::set_value(bool value)
{
	this->value = value;
}
Property::Type BoolProperty::type() const
{
	return BOOL;
}
void BoolProperty::copy(const Property& o)
{
	const BoolProperty* p = dynamic_cast<const BoolProperty*>(&o);
	if (p == NULL) {
		throw INVARG;
	}
	memcpy(this->name, p->name, sizeof(this->name));
	this->value = p->value;
}
bool BoolProperty::equals(const Property& o) const
{
	const BoolProperty* p = dynamic_cast<const BoolProperty*>(&o);
	if (p != NULL) {
		if (strcmp(this->name, p->name)==0) {
			if (this->value == p->value) {
				return true;
			}
		}
	}
	return false;
}
int BoolProperty::_str(char* buf) const//for debug
{
	return sprintf(buf, "(boolean)'%s'=>%s", this->name, (this->value ? "true" : "false"));
}






/**
 * Propertiesコンストラクタ
 */
Properties::Properties() : odc::PointerList<Property>(true)
{
}


/**
 * Propertyインスタンス追加。
 * @param[in] p Propertyインスタンス
 * @param[in] act 二重登録時動作
 * @return 処理結果
 * @retval 1 成功（上書き）
 * @retval 0 成功
 * @retval -1 失敗
 */
int Properties::add(Property* p, DupAct act)
{
	int result = 0;
	int idx = this->findi(p->get_name());
	if (idx >= 0) {
		switch (act) {
		case ALLOW:
			break;
		case DENY:
			return -1;
		case OVERWRITE:
			this->del(idx);
			result = 1;
			break;
		}
	}
	PointerList<Property>::add(p);
	return result;
}


/**
 * 指定プロパティ名の値取得。
 * 該当のプロパティが存在しなければNULLを返す。
 *
 * @attention
 * プロパティの型を限定しないため、意図した型ではないプロパティの値が取得されてしまう場合があります。
 * 型を特定して取得したい場合は、find_str()などの型指定の検索メソッドを利用して下さい。
 *
 * @param[in] name プロパティ名
 * @return 値へのポインタ
 */
const void* Properties::get_value(const char* name) const
{
	int idx = this->findi(name);
	if (idx >= 0) {
		const Property* p = this->get(idx);
		return p->get_value();
	}
	return NULL;
}


/**
 * 指定の名前のプロパティを検索する。
 * 該当するプロパティが存在しない場合は-1を返す。
 * @param[in] name プロパティ名
 * @param[in] stidx 検索開始インデックス
 * @return 検索結果（該当項目のインデックス、または-1）
 */
int Properties::findi(const char* name, unsigned stidx) const
{
	unsigned i;
	for (i=stidx ; i<this->size() ; i++) 	{
		if (this->memp[i]->is_name(name)) {
			return i;
		}
	}
	return -1;
}

/**
 * 指定の名前のプロパティを検索する。
 * 該当するプロパティが存在しない場合はNULLを返す。
 * @param[in] name プロパティ名
 * @return プロパティへのポインタ
 */
const Property* Properties::find(const char* name) const
{
	int idx = this->findi(name);
	if (idx >= 0) {
		return this->get(idx);
	}
	return NULL;
}


/**
 * 指定の名前の文字列型プロパティを検索する。
 * 該当する文字列型プロパティが存在しない場合はNULLを返す。
 * @attention
 * 	プロパティ名が同じでも、型が違う場合はNULLを返します。
 * @param[in] name プロパティ名
 * @return 値へのポインタ
 */
const char* Properties::find_str(const char* name) const
{
	const StringProperty* p = dynamic_cast<const StringProperty*>(this->find(name));
	if (p) {
		return static_cast<const char*>(p->get_value());
	} else {
		TRC("'%s' property not found.", name);
		return NULL;
	}
}

/**
 * 指定の名前の整数型プロパティを検索する。
 * 該当する整数型プロパティが存在しない場合はNULLを返す。
 * @attention
 * 	プロパティ名が同じでも、型が違う場合はNULLを返します。
 * @param[in] name プロパティ名
 * @return 値へのポインタ
 */
const int* Properties::find_int(const char* name) const
{
	const IntegerProperty* p = dynamic_cast<const IntegerProperty*>(this->find(name));
	if (p) {
		return static_cast<const int*>(p->get_value());
	} else {
		TRC("'%s' property not found.", name);
		return NULL;
	}
}

/**
 * 指定の名前の浮動小数型プロパティを検索する。
 * 該当する浮動小数型プロパティが存在しない場合はNULLを返す。
 * @attention
 * 	プロパティ名が同じでも、型が違う場合はNULLを返します。
 * @param[in] name プロパティ名
 * @return 値へのポインタ
 */
const double* Properties::find_float(const char* name) const
{
	const FloatProperty* p = dynamic_cast<const FloatProperty*>(this->find(name));
	if (p) {
		return static_cast<const double*>(p->get_value());
	} else {
		TRC("'%s' property not found.", name);
		return NULL;
	}
}

/**
 * 指定の名前のバイト列型プロパティを検索する。
 * 該当するバイト列型プロパティが存在しない場合はNULLを返す。
 * @attention
 * 	プロパティ名が同じでも、型が違う場合はNULLを返します。
 * @param[in] name プロパティ名
 * @return 値へのポインタ
 */
const unsigned char* Properties::find_byte(const char* name) const
{
	const CharProperty* p = dynamic_cast<const CharProperty*>(this->find(name));
	if (p) {
		return static_cast<const unsigned char*>(p->get_value());
	} else {
		TRC("'%s' property not found.", name);
		return NULL;
	}
}

/**
 * 指定の名前のブール型プロパティを検索する。
 * 該当するブール型プロパティが存在しない場合はNULLを返す。
 * @attention
 * 	プロパティ名が同じでも、型が違う場合はNULLを返します。
 * @param[in] name プロパティ名
 * @return 値へのポインタ
 */
const bool* Properties::find_bool(const char* name) const
{
	const BoolProperty* p = dynamic_cast<const BoolProperty*>(this->find(name));
	if (p) {
		return static_cast<const bool*>(p->get_value());
	} else {
		TRC("'%s' property not found.", name);
		return NULL;
	}
}




bool Properties::has(const Property& p) const
{
	static struct : public Matcher {
		const Property* prop;
		virtual bool match(const void* o) {
			const Property* p = reinterpret_cast<const Property*>(o);
			if (*this->prop == *p) {
				return true;
			}
			return false;
		}
	} m;

	m.prop = &p;
	int idx = PointerList<Property>::find(m);
	if (idx >= 0) {
		return true;
	}
	return false;
}


int Properties::_str(char* buf) const
{
	int n = this->size();
	int off = 0;
	for (unsigned int i=0 ; i<n ; i++) {
		if (i>0) {
			off+=sprintf(&buf[off], ", ");
		}
		Property* p = this->get(i);
		int rc = p->_str(&buf[off]);
		off += rc;
	}
	return off;
}







PropertiesHolder::PropertiesHolder() : odc::PointerList<odc::Properties>(true)
{

}


/**
 * 指定のプロパティを持つプロパティホルダーを検索する。
 * 該当するプロパティホルダーが存在しない場合は-1を返す。
 * @param[in] key プロパティ
 * @param[in] stidx 検索開始インデックス
 * @return 検索結果（該当項目のインデックス、または-1）
 */
int PropertiesHolder::findi(const Property& key, unsigned stidx) const
{
	unsigned i;
	for (i=stidx ; i<this->size() ; i++) 	{
		if (this->memp[i]->has(key)) {
			return i;
		}
	}
	return -1;
}



/**
 * 指定のプロパティを持つプロパティホルダーを検索する。
 * 該当するプロパティホルダーが存在しない場合はNULLを返す。
 * @param[in] key プロパティ
 * @return プロパティホルダーへのポインタ
 */
const Properties* PropertiesHolder::find(const Property& key) const
{
	int idx = this->findi(key);
	if (idx >= 0) {
		return this->get(idx);
	}
	return NULL;
}



void PropertiesHolder::_trace(int level) const
{
	int n = this->size();
	char buf[4096];
	for (unsigned int i=0 ; i<n ; i++) {
		Properties* p = this->get(i);
		p->_str(buf);
		TRACE((enum ODC_Trace_Level)level, 0, "#%d: %s", buf);
	}
}





/**
 * Chainコンストラクタ
 */
Chain::Chain()
{
	this->_next=this;
	this->_prev=this;
}

/**
 * 次挿入
 * このChainの次にpを追加する。
 * @param[in] p 挿入対象
 */
void Chain::addnext(Chain* p)
{
	Chain* w = this->_next;
	this->_next = p;
	p->_prev = this;
	p->_next = w;
	w->_prev = p;
}

/**
 * 前挿入
 * このChainの前にpを追加する。
 * @param[in] p 挿入対象
 */
void Chain::addprev(Chain* p)
{
	Chain* w = this->_prev;
	this->_prev = p;
	p->_next = this;
	p->_prev = w;
	w->_next = p;
}

/**
 * 取り出し
 * このChainを連結から取り外す。
 */
void Chain::remove()
{
	this->_next->_prev = this->_prev;
	this->_prev->_next = this->_next;
	this->_next = this;
	this->_prev = this;
}





}; /* namespace libodc */

