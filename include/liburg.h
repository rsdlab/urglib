 
#pragma once

#ifndef LIBURG_API

#ifdef WIN32
#ifdef _WINDLL
#define LIBURG_API //__declspec(dllexport)
#else // _WINDLL
#define LIBURG_API //__declspec(dllimport)
#endif // _WINDLL
#else // WIN32
#define LIBURG_API 
#endif // ifdef WIN32

#endif //LIBURG_API

#ifdef __cplusplus
#include "UrgBase.h"


#if 0
#include "Kobuki.h"


namespace rt_net {
	class Kobuki;
	class KobukiArgument;
};

/**
 * @brief Kobukiインターフェースのオブジェクト作成
 *
 * @param arg KobukiArgumentインターフェースを実装したオブジェクトを入れる．（現バージョンではKobukiStringArgument）
 * @return rt_net::Kobuki クラスオブジェクトへのポインタ
 */
LIBKOBUKI_API rt_net::Kobuki* createKobuki(const rt_net::KobukiArgument &arg);
#endif
#endif

//#include "kobukicwrapper.h"
