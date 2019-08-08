#pragma once

#include "../ps4.h"


/*
*	Part interface, part static manager to maintain a simple scene api.
*
*	
*/
class Scene
{
	static Scene *curr;

	bool visible=true;

public:

	virtual ~Scene() {
		/*
		if (curr) {
			curr->leave();
			delete curr;
		}*/
	};


	static Scene * Get()
	{
		return curr;
	}

	static void Set(Scene *next)
	{
		if (!next) return;

		if (curr) {
			curr->leave();
			delete curr;
		}
		curr = next;
		curr->enter();
	}

	static void Set(Scene& next)
	{
		Set(&next);
	}

	static void Render()
	{
		if (curr && curr->visible)
			curr->render();
	}

	static void Show() { if (curr) curr->visible = true; }
	static void Hide() { if (curr) curr->visible = false; }


	virtual void enter() = 0;
	virtual void leave() = 0;
	virtual void render()= 0;


};




























