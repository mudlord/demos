
#ifndef __BOTTOMSCREEN_H
#define __BOTTOMSCREEN_H

#include <stdint.h>
#include <windows.h>

#define LENSW					90	  //Lens dimension
#define LENSZ					15	  //Magnification

//Lens class
class TLens
{
private:
	int Lens[LENSW][LENSW];	//Buffer with precalculated distorsion

public:
	TLens();							//Constructor

										//Apply lens effect on a buffer
	void Apply_Lens(uint32_t *Dest, uint32_t *Src, int Ox, int Oy);
};
void Init_Bottomscreen();
void Do_Bottomscreen(uint32_t* dstImage);

#endif