// mapvalue.cpp : コンソール アプリケーション用のエントリ ポイントの定義
//

#include "stdafx.h"
#include "mapvalue.h"

struct Hoge2 {
	int z;
	int x;
	int y;

	MAPVALUE_BEGIN()
		MV_VALUE(z)
		MV_VALUE(x)
		MV_VALUE(y)
	MAPVALUE_END()
};

struct Hoge {
	int x;
	int y;
	int z;
	Hoge2 obj;
	Hoge2* pobj;
	std::vector<int> array_int;
	std::vector<Hoge2> array_obj;

	MAPVALUE_BEGIN()
		MV_VALUE(x)
		MV_VALUE(y)
		MV_VALUE(z)
		MV_OBJ(obj)
		MV_OBJP(pobj)
		MV_ARRAY(array_int)
		MV_ARRAYOBJ(array_obj)
	MAPVALUE_END()
};

int main(int argc, char* argv[])
{
	Hoge h;
	h.pobj = &h.obj;

	mv_ini_write(&h, "C:\\test2.ini", "h");

	std::vector<int> v;
	v.resize(100);
	v.erase(&v[3]);
	v.erase(v.begin() + 3);

	return 0;
}
