#ifndef RDEFINE_H__
#define RDEFINE_H__


#ifdef BASE_EXPORTS
#define BASE_API __declspec(dllexport)
#else
#define BASE_API __declspec(dllimport)
#endif




#ifdef RQTCORE_EXPORTS
#define RQTCORE_API __declspec(dllexport)
#else
#define RQTCORE_API __declspec(dllimport)
#endif


#endif //RDEFINE_H__