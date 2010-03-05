#pragma once

#include <string>
#include <sstream>
#include <list>
#include <vector>
#include <assert.h>

// ini 
#include <windows.h> 

// mapvalue
class mapvalue
{
public:
	// 定義
	typedef std::string string;
	typedef std::vector<mapvalue*> array;

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

	// 作成
	template<class T>
	void copy_to(T* p, const char* name=""){
		p->to_mapvalue(*this, name, map_to_obj);
	}

	// 作成
	template<class T>
	void copy_from(T* p, const char* name=""){
		p->to_mapvalue(*this, name, obj_to_map);
	}

	// タイプ
			type					get_type()				{ return m_type; }; // タイプの取得
			type					set_type(type t)		{ return m_type = t; }; // タイプの取得

	// 名前
			string					get_name()				{ return m_name; }// 名前の取得
			string					set_name(string name)	{ return m_name = name; }// 名前の取得

	// 親の取得
			mapvalue*				parent()				{ return m_parent; }
			std::vector<mapvalue*>	parentlist();

	// コンテナアクセスメソッド
			size_t		size()						const	{ return m_array.size();	}
			void		push_back(mapvalue& r)				{ m_array.push_back(new mapvalue(r)); m_array.back()->m_parent = this;	}
			mapvalue&	operator[](int n)					{ return *m_array[n];		}
			mapvalue&	operator[](string n)				{ return findandinsert(n);	}

			mapvalue&	findandinsert(string name);

	// データアクセスメソッド
	#define VALUE(T) \
			T&		get		(T* p)	{ std::stringstream s(m_value); s >> *p; return *p;} \
			T		get		(T)		{ return get_##T();} \
			T		get_##T	()		{ T t; return get(&t); } \
			void	set		(T v)	{ std::stringstream s; s << v; m_value = s.str(); }	\
			void	set##T	(T v)	{ set(v); } \
			void	push_back(T v)	{ mapvalue r; r.set(v); std::stringstream s; s << m_array.size(); r.m_name = s.str(); r.m_type = type_value; push_back(r); }

		VALUE(int)
		VALUE(float)
		VALUE(string)
		VALUE(double)
	#undef ACCESS

	// コンストラクタ
	mapvalue(string name=""){
		m_type = type_none;
		m_name = name;
		m_parent = 0;
	}

	// コピーコンストラクタ
	//mapvalue(const mapvalue& r){
	//	m_type = r.m_type;
	//	m_name = r.m_name;
	//	m_value = r.m_value;
	//	m_array = r.m_array;
	//	m_parent = r.m_parent;
	//}

	// デストラクタ
	~mapvalue(){
		for(int i=0; i<m_array.size(); i++) {
			delete m_array[i];
		}
	}

private:
	// メンバ
	type m_type;
	string m_name;
	string m_value;
	array m_array;
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
	return v;
}

// 文字列での名前検索アクセス
inline mapvalue&	mapvalue::findandinsert(string name)
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
#define MAPVALUE_BEGIN()	void to_mapvalue(mapvalue& s, const char* name, mapvalue::copy copy){ s.set_name(name); s.set_type(mapvalue::type_object);
#define MV_VALUE(v)				s[#v].set_type(mapvalue::type_value);  if(copy == mapvalue::obj_to_map) s[#v].set(v); else s[#v].get(&v);
#define MV_OBJ(v)				s[#v].set_type(mapvalue::type_object); v.to_mapvalue(s[#v], #v, copy);
#define MV_OBJP(v)				s[#v].set_type(mapvalue::type_object); v->to_mapvalue(s[#v], #v, copy);
#define MV_ARRAY(v)				s[#v].set_type(mapvalue::type_array);  if(copy == mapvalue::obj_to_map){ for(int i=0; i<v.size();i++) { s[#v].push_back( v[i] ); } }									else { v.resize(s.size()); for(int i=0; i<v.size();i++) { s[i].get(&v[i]); } }
#define MV_ARRAYOBJ(v)			s[#v].set_type(mapvalue::type_array);  if(copy == mapvalue::obj_to_map){ for(int i=0; i<v.size();i++) { mapvalue j; v[i].to_mapvalue(j,#v,copy); s[#v].push_back(j); }}	else { v.resize(s.size()); for(int i=0; i<v.size();i++) { v[i].to_mapvalue(s[i], #v, copy); }  }
#define MAPVALUE_END()		}

std::string mv_ini_get_parents_path(mapvalue* p)
{
	std::vector<mapvalue*> parents = p->parentlist();
	std::string s;

	if( !parents.empty() ) {
		for(int i=0; i<parents.size()-1; i++) {
			if(i != 0) s += ".";
			s += parents[i]->get_name();
		}
	}

	return s;
}

// ini ファイルのリードライト
template <class T>
void mv_write_ini(T* t, char* filename, char* section)
{
	mapvalue m;
	m.copy_from(t,section);
	mv_write_ini(&m, filename, section, "");
}

void mv_write_ini(mapvalue* p, char* filename, char* section, char* path)
{
	mapvalue& m = *p;
	if(m.get_type() == mapvalue::type_none) {
		assert(0);
	}
	else if(m.get_type() == mapvalue::type_value) {
		std::string s = mv_ini_get_parents_path(&m);

		if( p->parent() != 0 ) {
			if( p->parent()->get_type() == mapvalue::type_array ) {
				s += "[";
				s += m.get_name();
				s += "]";
			}
			else if( s == "" ){
				s += m.get_name();
			}
			else {
				s += ".";
				s += m.get_name();
			}
		}
		else {
			s = m.get_name();
		}
		::WritePrivateProfileString(section, s.c_str(), m.get_string().c_str(), filename);
	}
	else {
		for(int i=0; i<m.size(); i++) {
			mv_write_ini(&m[i], filename, section );
		}
	}
}

// 読み込み
template <class T>
void mv_read_ini(T* t, char* filename, char* section)
{
	mapvalue m;
	m.copy_from(t,section);
	mv_read_ini(&m, filename, section);
	m.copy_to(t);
}

void mv_read_ini(mapvalue* p, char* filename, char* section)
{
	mapvalue& m = *p;
	if(m.get_type() == mapvalue::type_none) {
		assert(0);
	}
	else if(m.get_type() == mapvalue::type_value) {
		std::string s = mv_ini_get_parents_path(&m);
		s += m.get_name();

		char buf[512];
		::GetPrivateProfileString(section, s.c_str(), "", buf, 512, filename);
		m.set(buf);
	}
	else if(m.get_type() == mapvalue::type_array) {
		// 配列データ読み込み
		std::string s = mv_ini_get_parents_path(&m);
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
				m.push_back(buf);
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
