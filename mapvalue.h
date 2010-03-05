#pragma once

#include <string>
#include <sstream>
#include <list>
#include <vector>
#include <assert.h>
#include <algorithm>

// ini 
#include <windows.h> 

// mapvalue
class mapvalue
{
public:
	// オブジェクトタイプ
	enum type {
		type_none,
		type_value,
		type_array,
		type_object,
	};

	// 取得関数への引数
	enum copy {
		map_to_obj,
		obj_to_map,
	};

	// タイプ
	type					get_type()						{ return m_type; }; // タイプの取得
	type					set_type(type t)				{ return m_type = t; }; // タイプの取得

	// 名前
	std::string				get_name()						{ return m_name; }// 名前の取得
	std::string				set_name(std::string name)		{ return m_name = name; }// 名前の取得

	// 親の取得
	mapvalue*				parent()			{ return m_parent; }
	std::vector<mapvalue*>	parentlist();

	// 値型アクセス
	template<class T> T		get(T* p)			{ std::stringstream s(m_value); s >> *p; return *p;}
	template<class T> void	set(T v)			{ std::stringstream s; s << v; m_value = s.str(); }
	template<class T> void	push_back_value(T v){ mapvalue r; r.set(v); std::stringstream s; s << m_array.size(); r.m_name = s.str(); r.m_type = type_value; push_back(r); }

	// 配列及びオブジェクト型アクセスメソッド
	size_t					size()						const{ return m_array.size();	}
	void					push_back(mapvalue& r)			{ m_array.push_back(new mapvalue(r));	m_array.back()->m_parent = this;	}
	void					push_back(mapvalue* p)			{ m_array.push_back(p);					m_array.back()->m_parent = this;	}
	mapvalue&				operator[](int n)				{ return *m_array[n];		}
	mapvalue&				operator[](std::string n)		{ return findandinsert(n);	}


	// コンストラクタ
	mapvalue(std::string name="")							{ m_type = type_none; m_name = name; m_parent = 0;	}
	// デストラクタ
	~mapvalue(){
		for(int i=0; i<m_array.size(); i++) {
			delete m_array[i];
		}
	}

private:
	// 文字列での名前検索アクセス
	mapvalue&	findandinsert(std::string name);

	// メンバ
	type m_type;
	std::string m_name;
	std::string m_value;
	std::vector<mapvalue*> m_array;
	mapvalue* m_parent;
};

// 親リストの作成
inline std::vector<mapvalue*> mapvalue::parentlist(){
	std::vector<mapvalue*> v;
	mapvalue* p = this->parent();
	while(p != 0) {
		v.push_back(p);
		p = p->parent();
	}
	std::reverse(v.begin(), v.end());
	return v;
}

// 文字列での名前検索アクセス
inline mapvalue&	mapvalue::findandinsert(std::string name)
{
	for(int i=0;i< size(); i++) {
		if(m_array[i]->m_name == name) {
			return *m_array[i];
		}
	}
	m_array.push_back( new mapvalue(name) );
	m_array.back()->m_parent = this;
	return *m_array.back();
}

// マクロ
#define MAPVALUE_BEGIN()	void to_mapvalue(mapvalue* p, const char* name, mapvalue::copy copy){ mapvalue& s = *p; s.set_name(name); s.set_type(mapvalue::type_object);
#define		MV_VALUE(v)			mv_value(s, #v,  copy, v);
#define		MV_OBJ(v)			mv_obj(s, #v,  copy, v);
#define		MV_OBJP(v)			mv_obj(s, #v,  copy, *v);
#define		MV_ARRAY(v)			mv_array(s, #v,  copy, v);
#define		MV_ARRAYOBJ(v)		mv_arrayobj(s, #v,  copy, v);
#define MAPVALUE_END()		}

// 値指定
template<class T>
void mv_value(mapvalue& s, const char* name, mapvalue::copy copy, T& v)
{
	s[name].set_type(mapvalue::type_value);
	if(copy == mapvalue::obj_to_map) {
		s[name].set(v);
	}
	else{
		s[name].get(&v);
	}
}

template<class T>
void mv_obj(mapvalue& s, const char* name, mapvalue::copy copy, T& v)
{
	s[name].set_type(mapvalue::type_object);
	v.to_mapvalue(&s[name], name, copy);
}

template<class T>
void mv_array(mapvalue& s, const char* name, mapvalue::copy copy, T& v)
{
	s[name].set_type(mapvalue::type_array);
	if(copy == mapvalue::obj_to_map){
		for(int i=0; i<v.size();i++) {
			s[name].push_back_value( v[i] );
		}
	}
	else {
		v.resize(s.size());
		for(int i=0; i<v.size();i++) {
			s[i].get(&v[i]);
		}
	}
}
template<class T>
void mv_arrayobj(mapvalue& s, const char* name, mapvalue::copy copy, T& v)
{
	s[name].set_type(mapvalue::type_array);
	if(copy == mapvalue::obj_to_map){
		for(int i=0; i<v.size();i++) {
			mapvalue* p = new mapvalue();
			char buf[512];
			v[i].to_mapvalue(p,itoa(i,buf,512),copy);
			s[name].push_back(p);
		}
	}
	else {
		v.resize(s.size());
		for(int i=0; i<v.size();i++) {
			v[i].to_mapvalue(&s[i], name, copy);
		}
	}
}

std::string mv_ini_path(mapvalue* p)
{
	std::vector<mapvalue*> list = p->parentlist();
	list.push_back(p);
	std::string s;

	if( !list.empty() ) {
		bool bEndKakko=false;
		for(int i=1; i<list.size(); i++) {
			s += list[i]->get_name();
			if ( bEndKakko ) {
				s+="]";
				bEndKakko = false;
			}
			if(list[i]->get_type() == mapvalue::type_object) {
				s+=".";
			}
			else if (list[i]->get_type() == mapvalue::type_array) {
				s+="[";
				bEndKakko = true;
			}
		}
	}

	return s;
}

// ini ファイルのリードライト
template <class T>
void mv_write_ini(T* t, char* filename, char* section)
{
	mapvalue m;
	t->to_mapvalue(&m, section, mapvalue::obj_to_map);
	mv_write_ini_path(&m, filename, section, "");
}

void mv_write_ini_path(mapvalue* p, char* filename, char* section, char* path)
{
	mapvalue& m = *p;
	if(m.get_type() == mapvalue::type_none) {
		assert(0);
	}
	else if(m.get_type() == mapvalue::type_value) {
		std::string s = mv_ini_path(&m);
		::WritePrivateProfileString(section, s.c_str(), m.get(&std::string()).c_str(), filename);
	}
	else {
		for(int i=0; i<m.size(); i++) {
			mv_write_ini_path(&m[i], filename, section, path );
		}
	}
}

// 読み込み
template <class T>
void mv_read_ini(T* t, char* filename, char* section)
{
	mapvalue m;
	t->to_mapvalue(&m, section, mapvalue::obj_to_map);
	mv_read_ini(&m, filename, section);
	t->to_mapvalue(&m, section, mapvalue::map_to_obj);
}

void mv_read_ini(mapvalue* p, char* filename, char* section)
{
	mapvalue& m = *p;
	if(m.get_type() == mapvalue::type_none) {
		assert(0);
	}
	else if(m.get_type() == mapvalue::type_value) {
		std::string s = mv_ini_path(&m);

		char buf[512];
		::GetPrivateProfileString(section, s.c_str(), "", buf, 512, filename);
		m.set(buf);
	}
	else if(m.get_type() == mapvalue::type_array) {
		// 配列データ読み込み
		std::string s = mv_ini_path(&m);
		s += m.get_name();
		char buf[512];
		char nbuf[512];
		int nCnt=0;
		do {
			std::string key = s;
			key += "[";
			key += itoa(nCnt, nbuf, 512);
			key += "]";
			::GetPrivateProfileString(section, key.c_str(), "", buf, 512, filename);
			if(buf[0] != '\0') {
				m.push_back_value(buf);
			}
			nCnt++;
		} while(buf[0] != '\0');
	}
	else if(m.get_type() == mapvalue::type_object) {
		// オブジェクトの読み込み
		for(int i=0; i<m.size(); i++) {
			mv_read_ini(&m[i], filename, section );
		}
	}
}
