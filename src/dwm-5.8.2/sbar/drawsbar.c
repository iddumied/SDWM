void drawsbar(Monitor *m, unsigned int occ);

void drawsbar(Monitor *m, unsigned int occ){
	unsigned int i;
	
	XCopyArea(dpy, sbars[ab].drawable , dc.drawable, dc.gc, 0,0,sbars[ab].w,sbars[ab].h,0,0);
	
	for(i = 0; i < sbars[ab].ntabs; i++)
	{  
		if(m->tagset[m->seltags] & 1 << i) // active tag
		{
			if(m == selmon && selmon->sel && selmon->sel->tags & 1 << i){ // active tag with windows
				XCopyArea(dpy, sbars[ab].tabs[i].full_active , dc.drawable, dc.gc,
						0, 0, sbars[ab].tabs[i].w,sbars[ab].tabs[i].w, 
						sbars[ab].tabs[i].pos_x, sbars[ab].tabs[i].pos_y);
				
			}else{ // active tag without windows
				XCopyArea(dpy, sbars[ab].tabs[i].empty_active , dc.drawable, dc.gc,
						0, 0, sbars[ab].tabs[i].w,sbars[ab].tabs[i].w, 
						sbars[ab].tabs[i].pos_x, sbars[ab].tabs[i].pos_y);
			}
		}else{ // not active tag
			if(m == selmon && selmon->sel && selmon->sel->tags & 1 << i){ // tag with active window
				XCopyArea(dpy, sbars[ab].tabs[i].has_activ_win , dc.drawable, dc.gc,
						0, 0, sbars[ab].tabs[i].w,sbars[ab].tabs[i].w, 
						sbars[ab].tabs[i].pos_x, sbars[ab].tabs[i].pos_y);
				
			}else{
				if(occ & 1 << i){ // tag with windows
					XCopyArea(dpy, sbars[ab].tabs[i].has_win , dc.drawable, dc.gc,
							0, 0, sbars[ab].tabs[i].w,sbars[ab].tabs[i].w, 
							sbars[ab].tabs[i].pos_x, sbars[ab].tabs[i].pos_y);
				}
			}
		}
	}
}