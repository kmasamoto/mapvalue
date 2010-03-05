#pragma once

#include <string>
#include <sstream>
#include <vector>

#ifndef for_
	#define for_ if(0);else for
#endif

class mapvalue
{
public:
	// 定義
	typedef std::string string;
	typedef std::vector<mapvalue> array;

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

	// タイプの取得
	type get_type(){ return m_type; };

	// 親の取得
	mapvalue* parent(){ }

	// コンテナアクセスメソッド
			size_t		size()						const	{ return m_array.size();	}
			void		push_back(mapvalue r)				{ m_array.push_back(r);	}
			mapvalue&	operator[](int n)					{ return m_array[n];		}
			mapvalue&	operator[](string n)				{ m_type = type_object; return findandinsert(n);	}

	mapvalue& findandinsert(string name)
	{
		for(int i=0;i< size(); i++) {
			if(m_array[i].m_name == name) {
				return m_array[i];
			}
		}
		m_array.resize(m_array.size()+1);
		m_array.back().m_name = name;
		return m_array.back();
	}

	// データアクセスメソッド
	#define VALUE(T) \
			T&		get		(T* p)	{ std::stringstream s(m_value); s >> *p; return *p;} \
			T		get_##T	()		{ std::stringstream s(m_value); T v; (s >> v); return v; } \
			void	set		(T v)	{ std::stringstream s(m_value); s << v; m_type = type_value; }	\
			void	set##T	(T v)	{ set(v); m_type = type_value;}

		VALUE(int)
		VALUE(float)
		VALUE(string)
		VALUE(double)
	#undef ACCESS

	// コンストラクタ
	mapvalue(){
		m_type = type_none;
	}
	// コピーコンストラクタ
	mapvalue(const mapvalue& r){
		m_type = r.m_type;
		m_name = r.m_name;
		m_value = r.m_value;
		m_array = r.m_array;
	}
	~mapvalue(){
	}

private:
	type m_type;
	string m_name;
	string m_value;
	array m_array;
	mapvalue* m_parent;
};

#define MAPVALUE_BEGIN()	void to_mapvalue(mapvalue& s, const char* name, mapvalue::getset getset){
#define MV_VALUE(v)				if(getset = mapvalue::setmapvalue) s[#v].set(v); else s[#v].get(&v);
#define MV_OBJ(v)				v.to_mapvalue(s[#v], #v, getset);
#define MV_OBJP(v)				v->to_mapvalue(s[#v], #v, getset);
#define MV_ARRAY(v)				for_(int i=0; i<v.size();i++) { s[#v].push_back(v[i]); }
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

template <class T>
void mv_ini_write(T* t, char* filename, char* section)
{
	mapvalue m;
	t->to_mapvalue(m, name, mapvalue::get);

	mv_ini_write(&m, filename, section);
}

void mv_ini_write(mapvalue* m)
{

}
