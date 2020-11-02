/*
** TinyPTC by Gaffer
** GDI port by Zoon, cleanified by 8bitbubsy
*/

#ifndef __TINYPTC_GDI_H
#define __TINYPTC_GDI_H

#ifdef __cplusplus
extern "C" {
#endif
	int ptcOpen(const char *title, short width, short height);
	int ptcUpdate(void *buffer);
	void ptcClose(void);

#ifdef __cplusplus
}
#endif


#endif
