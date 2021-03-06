/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include	<sfio/sfhdr.h>

/*	Seek function that knows discipline
**
**	Written by Kiem-Phong Vo.
*/
Sfoff_t sfsk(Sfio_t * f, Sfoff_t addr, int type, Sfdisc_t * disc)
{
    Sfoff_t p;
    Sfdisc_t *dc;
    ssize_t s;
    int local, mode;

    SFMTXSTART(f, (Sfoff_t) (-1));

    GETLOCAL(f, local);
    if (!local && !(f->bits & SF_DCDOWN)) {
	if ((mode = f->mode & SF_RDWR) != (int) f->mode
	    && _sfmode(f, mode, 0) < 0)
	    SFMTXRETURN(f, (Sfoff_t) (-1));
	if (SFSYNC(f) < 0)
	    SFMTXRETURN(f, (Sfoff_t) (-1));
	f->next = f->endb = f->endr = f->endw = f->data;
    }

    if ((type &= (SEEK_SET | SEEK_CUR | SEEK_END)) > SEEK_END)
	SFMTXRETURN(f, (Sfoff_t) (-1));

    for (;;) {
	dc = disc;
	if (f->flags & SF_STRING) {
	    SFSTRSIZE(f);
	    if (type == SEEK_SET)
		s = (ssize_t) addr;
	    else if (type == SEEK_CUR)
		s = (ssize_t) (addr + f->here);
	    else
		s = (ssize_t) (addr + f->extent);
	} else {
	    SFDISC(f, dc, seekf);
	    if (dc && dc->seekf) {
		SFDCSK(f, addr, type, dc, p);
	    } else {
		p = lseek(f->file, (off_t) addr, type);
	    }
	    if (p >= 0)
		SFMTXRETURN(f, p);
	    s = -1;
	}

	if (local)
	    SETLOCAL(f);
	switch (_sfexcept(f, SF_SEEK, s, dc)) {
	case SF_EDISC:
	case SF_ECONT:
	    if (f->flags & SF_STRING)
		SFMTXRETURN(f, (Sfoff_t) s);
	    goto do_continue;
	default:
	    SFMTXRETURN(f, (Sfoff_t) (-1));
	}

      do_continue:
	for (dc = f->disc; dc; dc = dc->disc)
	    if (dc == disc)
		break;
	disc = dc;
    }
}
