#ifndef	VCSspark_API_H
#define VCSspark_API_H


#ifdef VCS_API_EXPORTS 
#define VCS_API __declspec(dllexport)
#else
#define VCS_API
#endif

#ifndef IN
#define	IN
#endif

#ifndef OUT
#define	OUT
#endif

#ifndef INOUT
#define	INOUT
#endif

#ifdef __cplusplus
extern "C" {
#endif 
	
	

	
#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !VCS_API_H
