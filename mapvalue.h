#pragma once

#include <string>
#include <sstream>
#include <list>
#include <vector>
#include <assert.h>

#ifndef for_
	#define for_ if(0);else for
#endif

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
	enum getset {
		getmapvalue,
		setmapvalue,
	};

	
			type					get_type()				{ return m_type; }; // タイプの取得
			type					set_type(type t)		{ return m_type = t; }; // タイプの取得
			string					get_name()				{ return m_name; }// 名前の取得
			string					set_name(string name)	{ return m_name = name; }// 名前の取得

	// 親の取得
			mapvalue*				parent()				{ return m_parent; }
			std::vector<mapvalue*>	parentlist();

	// コンテナアクセスメソッド
			size_t		size()						const	{ return m_array.size();	}
			void		push_back(mapvalue r)				{ m_array.push_back(new mapvalue(r)); m_array.back()->m_parent = this;	}
			mapvalue&	operator[](int n)					{ return *m_array[n];		}
			mapvalue&	operator[](string n)				{ m_type = type_object; return findandinsert(n);	}

			mapvalue&	findandinsert(string name);

	// データアクセスメソッド
	#define VALUE(T) \
			T&		get		(T* p)	{ std::stringstream s(m_value); s >> *p; return *p;} \
			T		get_##T	()		{ std::stringstream s(m_value); T v; (s >> v); return v; } \
			void	set		(T v)	{ std::stringstream s; s << v; m_value = s.str(); m_type = type_value; }	\
			void	set##T	(T v)	{ set(v); } \
			void	push_back(T v)	{ mapvalue r; r.set(v); push_back(r); }

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
	mapvalue(const mapvalue& r){
		m_type = r.m_type;
		m_name = r.m_name;
		m_value = r.m_value;
		m_array = r.m_array;
		m_parent = r.m_parent;
	}
	~mapvalue(){
		for(int i=0; i<m_array.size(); i++) {
			delete m_array[i];
		}
	}

private:
	type m_type;
	string m_name;
	string m_value;
	array m_array;
	mapvalue* m_parent;
};

inline std::vector<mapvalue*> mapvalue::parentlist(){
	std::vector<mapvalue*> v;
	mapvalue* p = this->parent();
	while(p != 0) {
		v.push_back(p);
		p = p->parent();
	}
	return v;
}

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

#define MAPVALUE_BEGIN()	void to_mapvalue(mapvalue& s, const char* name, mapvalue::getset getset){ s.set_name(name);
#define MV_VALUE(v)				if(getset = mapvalue::setmapvalue) s[#v].set(v); else s[#v].get(&v);
#define MV_OBJ(v)				v.to_mapvalue(s[#v], #v, getset);
#define MV_OBJP(v)				v->to_mapvalue(s[#v], #v, getset);
#define MV_ARRAY(v)				for_(int i=0; i<v.size();i++) { s[#v].push_back( v[i] ); }
#define MV_ARRAYOBJ(v)			for_(int i=0; i<v.size();i++) { mapvalue j; v[i].to_mapvalue(j,#v,getset); s[#v].push_back(j); }
#define MAPVALUE_END()		}

template <class T>
void mapvalue_read(T* t, char* filename, char* name)
{
	mapvalue j;
	j.read(filename);
	std::string s = name;
	t->to_mapvalue(j[s], s.c_str(), mapvalue::setmapvalue);
}
template <class T>
void mapvalue_write(T* t, char* filename, char* name)
{
	mapvalue j;
	t->to_mapvalue(j[name], name, mapvalue::getmapvalue);
	j.write(filename);
}

#include <windows.h> 

template <class T>
void mv_ini_write(T* t, char* filename, char* section)
{
	mapvalue m;
	t->to_mapvalue(m, section, mapvalue::getmapvalue);

	mv_ini_write(&m, filename, section);
}

void mv_ini_write(mapvalue* p, char* filename, char* section)
{
	mapvalue& m = *p;
	if(m.get_type() == mapvalue::type_none) {
		assert(0);
	}
	else if(m.get_type() == mapvalue::type_value) {
		std::vector<mapvalue*> parents = m.parentlist();
		std::string s;
		for(int i=0; i<parents.size(); i++) {
			s += parents[i]->get_name();
			s += ".";
		}
		s += m.get_name();
		::WritePrivateProfileString(section, s.c_str(), m.get_string().c_str(), filename);
	}
	else {
		for(int i=0; i<m.size(); i++) {
			mv_ini_write(&m[i], filename, section );
		}
	}
}
