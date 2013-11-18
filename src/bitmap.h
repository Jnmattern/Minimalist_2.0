#define TRIG_MAX (1<<24)
#define TRIG_NORM(v) ((v)>>24)
#define TRIG_MULT(v) ((v)<<24)

/*\
|*| bmpDrawArc function thanks to Cameron MacFarland (http://forums.getpebble.com/profile/12561/Cameron%20MacFarland)
\*/

const int32_t _sin_[91] = {
	0, 292802, 585516, 878051, 1170319, 1462230, 1753696, 2044628, 2334937, 2624534, 2913332, 3201243, 3488179, 3774052,
	4058775, 4342263, 4624427, 4905183, 5184444, 5462127, 5738145, 6012416, 6284855, 6555380, 6823908, 7090357, 7354647,
	7616696, 7876425, 8133755, 8388607, 8640905, 8890569, 9137526, 9381700, 9623015, 9861400, 10096780, 10329085, 10558244,
	10784186, 11006844, 11226148, 11442033, 11654433, 11863283, 12068519, 12270079, 12467901, 12661925, 12852093, 13038345,
	13220626, 13398880, 13573052, 13743090, 13908942, 14070557, 14227886, 14380880, 14529495, 14673683, 14813402, 14948608,
	15079261, 15205321, 15326749, 15443508, 15555563, 15662880, 15765426, 15863169, 15956080, 16044131, 16127295, 16205546,
	16278860, 16347217, 16410593, 16468971, 16522332, 16570660, 16613941, 16652161, 16685308, 16713373, 16736347, 16754223,
	16766995, 16774660, 16777216
};


static inline int32_t _SIN(int d) {
    d = d%360;
    if (d < 90) {
        return _sin_[d];
    } else if (d < 180) {
        return _sin_[180-d];
    } else if (d < 270) {
        return -_sin_[d-180];
    } else {
        return -_sin_[360-d];
    }
}

static inline int32_t _COS(int d) {
    d = d%360;
    if (d < 90) {
        return _sin_[90-d];
    } else if (d < 180) {
        return -_sin_[d-90];
    } else if (d < 270) {
        return -_sin_[270-d];
    } else {
        return _sin_[d-270];
    }
}

static inline GColor bmpGetPixel(const GBitmap *bmp, int x, int y) {
    if (x >= bmp->bounds.size.w || y >= bmp->bounds.size.h || x < 0 || y < 0) return -1;
    int byteoffset = y*bmp->row_size_bytes + x/8;
    return ((((uint8_t *)bmp->addr)[byteoffset] & (1<<(x%8))) != 0);
}

static inline void bmpPutPixel(GBitmap *bmp, int x, int y, GColor c) {
    if (x >= bmp->bounds.size.w || y >= bmp->bounds.size.h || x < 0 || y < 0) return;
    int byteoffset = y*bmp->row_size_bytes + x/8;
    ((uint8_t *)bmp->addr)[byteoffset] &= ~(1<<(x%8));
    if (c == GColorWhite) ((uint8_t *)bmp->addr)[byteoffset] |= (1<<(x%8));
}

static void bmpNegative(GBitmap *bmp) {
    for (int i=0; i<bmp->row_size_bytes*bmp->bounds.size.h; i++) ((uint8_t *)bmp->addr)[i] = ~((uint8_t *)bmp->addr)[i];
}

static void bmpCopy(const GBitmap *src, GBitmap *dst) {
    for (int y=0; y<dst->bounds.size.h; y++) {
        for (int x=0; x<dst->bounds.size.w; x++) {
            bmpPutPixel(dst, x, y, bmpGetPixel(src, x, y));
        }
    }
}

static void bmpSub(const GBitmap *src, GBitmap *dst, GRect from, GPoint to) {
    for (int y=0; y<from.size.h; y++) {
        for (int x=0; x<from.size.w; x++) {
            bmpPutPixel(dst, to.x+x, to.y+y, bmpGetPixel(src, from.origin.x+x, from.origin.y+y));
        }
    }
}

static inline void bmpFillRect(GBitmap *bmp, GRect rect, GColor c) {
    int i, j;
    int xe = rect.origin.x + rect.size.w;
    int ye = rect.origin.y + rect.size.h;
 
    for (j=rect.origin.y ; j<ye; j++) {
        for (i=rect.origin.x ; i<xe; i++) {
            bmpPutPixel(bmp, i, j, c);
        }
    }    
}

static inline void bmpFill(GBitmap *bmp, GColor c) {
	int i, l = bmp->row_size_bytes*bmp->bounds.size.h;
	uint8_t p = 0xff * c;
	uint8_t *d = bmp->addr;
	
	for (i=0; i<l; i++) {
		d[i] = p;
	}
}


static void bmpDrawLine(GBitmap *bmp, GPoint p1, GPoint p2, GColor c) {
	int dx, dy, e;
    
	if ((dx = p2.x-p1.x) != 0) {
		if (dx > 0) {
			if ((dy = p2.y-p1.y) != 0) {
				if (dy > 0) {
					// vecteur oblique dans le 1er quadran
					if (dx >= dy) {
						// vecteur diagonal ou oblique proche de l’horizontale, dans le 1er octant
						e = dx;
						dx = 2*e;
						dy = 2*dy;
						while (1) {
							bmpPutPixel(bmp, p1.x, p1.y, c);
							p1.x++;
							if (p1.x == p2.x) break;
							e -= dy;
							if (e < 0) {
								p1.y++;
								e += dx;
							}
						}
					} else {
						// vecteur oblique proche de la verticale, dans le 2nd octant
						e = dy;
						dy = 2*e;
						dx = 2*dx;
						while (1) {
							bmpPutPixel(bmp, p1.x, p1.y, c);
							p1.y++;
							if (p1.y == p2.y) break;
							e -= dx;
							if (e < 0) {
								p1.x++;
								e += dy;
							}
						}
					}
				} else { // dy < 0 (et dx > 0)
					// vecteur oblique dans le 4e cadran
					if (dx >= -dy) {
						// vecteur diagonal ou oblique proche de l’horizontale, dans le 8e octant
						e = dx;
						dx = 2*e;
						dy = 2*dy;
						while (1) {
							bmpPutPixel(bmp, p1.x, p1.y, c);
							p1.x++;
							if (p1.x == p2.x) break;
							e += dy;
							if (e < 0) {
								p1.y--;
								e += dx;
							}
						}
					} else {
						// vecteur oblique proche de la verticale, dans le 7e octant
						e = dy;
						dy = 2*e;
						dx = 2*dx;
						while (1) {
							bmpPutPixel(bmp, p1.x, p1.y, c);
							p1.y--;
							if (p1.y == p2.y) break;
							e += dx;
							if (e > 0) {
								p1.x++;
								e += dy;
							}
						}
					}
				}
			} else {
				// dy = 0 (et dx > 0)
                // vecteur horizontal vers la droite
				while (1) {
					bmpPutPixel(bmp, p1.x, p1.y, c);
					p1.x++;
					if (p1.x == p2.x) break;
				}
			}
		} else {
			// dx < 0
			if ((dy = p2.y-p1.y) != 0) {
				if (dy > 0) {
					// vecteur oblique dans le 2nd quadran
					if (-dx >= dy) {
						// vecteur diagonal ou oblique proche de l’horizontale, dans le 4e octant
						e = dx;
						dx = 2*e;
						dy = 2*dy;
						while (1) {
							bmpPutPixel(bmp, p1.x, p1.y, c);
							p1.x--;
							if (p1.x == p2.x) break;
							e += dy;
							if (e >= 0) {
								p1.y++;
								e += dx;
							}
						}
					} else {
						// vecteur oblique proche de la verticale, dans le 3e octant
						e = dy;
						dy = 2*e;
						dx = 2*dx;
						while (1) {
							bmpPutPixel(bmp, p1.x, p1.y, c);
							p1.y++;
							if (p1.y == p2.y) break;
							e += dx;
							if (e <= 0) {
								p1.x--;
								e += dy;
							}
						}
					}
				} else {
					// dy < 0 (et dx < 0)
					// vecteur oblique dans le 3e cadran
                    if (dx <= dy) {
						// vecteur diagonal ou oblique proche de l’horizontale, dans le 5e octant
						e = dx;
						dx = 2*e;
						dy = 2*dy;
						while (1) {
							bmpPutPixel(bmp, p1.x, p1.y, c);
							p1.x--;
							if (p1.x == p2.x) break;
							e -= dy;
							if (e >= 0) {
								p1.y--;
								e += dx;
							}
						}
					} else {
						// vecteur oblique proche de la verticale, dans le 6e octant
						e = dy;
						dy = 2*e;
						dx = 2*dx;
						while (1) {
							bmpPutPixel(bmp, p1.x, p1.y, c);
							p1.y--;
							if (p1.y == p2.y) break;
							e -= dx;
							if (e >= 0) {
								p1.x--;
								e += dy;
							}
						}
					}
				}
			} else {
				// dy = 0 (et dx < 0)
				// vecteur horizontal vers la gauche
				while (1) {
					bmpPutPixel(bmp, p1.x, p1.y, c);
					p1.x--;
					if (p1.x == p2.x) break;
				}
			}
		}
	} else {
		// dx = 0
		if ((dy = p2.y-p1.y) != 0) {
			if (dy > 0) {
				// vecteur vertical croissant
				while (1) {
					bmpPutPixel(bmp, p1.x, p1.y, c);
					p1.y++;
					if (p1.y == p2.y) break;
				}
			} else {
				// dy < 0 (et dx = 0)
				// vecteur vertical décroissant
				while (1) {
					bmpPutPixel(bmp, p1.x, p1.y, c);
					p1.y--;
					if (p1.y == p2.y) break;
				}
            }
		}
	}
}
/*\
|*| bmpDrawArc function thanks to Cameron MacFarland (http://forums.getpebble.com/profile/12561/Cameron%20MacFarland)
\*/
static void bmpDrawArc(GBitmap *bmp, GPoint center, int r, int t, int s, int e, GColor c) {
	s = s%360;
	e = e%360;
 
	while (s < 0) s += 360;
	while (e < 0) e += 360;
 
	if (e == 0) e = 360;

	float sslope = (float)_COS(s) / (float)_SIN(s);
	float eslope = (float)_COS(e) / (float)_SIN(e);
 
	if (e == 360) eslope = -1000000;
 
	int ir2 = (r - t) * (r - t);
	int or2 = r * r;
 
	for (int x = -r; x <= r; x++) {
		for (int y = -r; y <= r; y++)
		{
			int x2 = x * x;
			int y2 = y * y;
 
			if (
				(x2 + y2 < or2 && x2 + y2 >= ir2) && (
					(y > 0 && s < 180 && x <= y * sslope) ||
					(y < 0 && s > 180 && x >= y * sslope) ||
					(y < 0 && s <= 180) ||
					(y == 0 && s <= 180 && x < 0) ||
					(y == 0 && s == 0 && x > 0)
				) && (
					(y > 0 && e < 180 && x >= y * eslope) ||
					(y < 0 && e > 180 && x <= y * eslope) ||
					(y > 0 && e >= 180) ||
					(y == 0 && e >= 180 && x < 0) ||
					(y == 0 && s == 0 && x > 0)
				)
			)
			bmpPutPixel(bmp, center.x+x, center.y+y, c);
		}
	}
}

static void bmpDrawCircle(GBitmap *bmp, GPoint center, int r, GColor c) {
	int x = 0, y = r, d = r-1;
    
	while (y >= x) {
		bmpPutPixel(bmp, center.x+x, center.y+y, c);
		bmpPutPixel(bmp, center.x+y, center.y+x, c);
		bmpPutPixel(bmp, center.x-x, center.y+y, c);
		bmpPutPixel(bmp, center.x-y, center.y+x, c);
		bmpPutPixel(bmp, center.x+x, center.y-y, c);
		bmpPutPixel(bmp, center.x+y, center.y-x, c);
		bmpPutPixel(bmp, center.x-x, center.y-y, c);
		bmpPutPixel(bmp, center.x-y, center.y-x, c);
        
		if (d >= 2*x-2) {
			d = d-2*x;
			x++;
		} else if (d <= 2*r - 2*y) {
			d = d+2*y-1;
			y--;
		} else {
			d = d + 2*y - 2*x - 2;
			y--;
			x++;
		}
	}
}

static void bmpFillCircle(GBitmap *bmp, GPoint center, int r, GColor c) {
	int x = 0, y = r, d = r-1, v;
    
	while (y >= x) {
        for (v=center.x-x; v<=center.x+x; v++) bmpPutPixel(bmp, v, center.y+y, c);
        for (v=center.x-y; v<=center.x+y; v++) bmpPutPixel(bmp, v, center.y+x, c);
        for (v=center.x-x; v<=center.x+x; v++) bmpPutPixel(bmp, v, center.y-y, c);
        for (v=center.x-y; v<=center.x+y; v++) bmpPutPixel(bmp, v, center.y-x, c);
        
		if (d >= 2*x-2) {
			d = d-2*x;
			x++;
		} else if (d <= 2*r - 2*y) {
			d = d+2*y-1;
			y--;
		} else {
			d = d + 2*y - 2*x - 2;
			y--;
			x++;
		}
	}
}

#define MIN(a, b) (((a)<(b))?(a):(b))
#define MAX(a, b) (((a)<(b))?(b):(a))

static void bmpRotate(const GBitmap *src, GBitmap *dst, int a, GRect *srcClipRect, GPoint srcCenter, GPoint dstOffset) {
	int i;
	int x, y, c;
	int xmin = 100000, xmax = -100000, ymin = 100000, ymax = -100000;
	int32_t cosphi, sinphi, xc, xs, yc, ys, xo, yo, xx, yy;
	GPoint srcCorner[4], dstCorner[4];

	// Normalize angle so that 0 <= a < 360
	while (a<0) {
		a += 360;
	}
	a = a%360;

	// Compute the 4 corners' coordinates of the src bitmap (or clipping rect if provided) once rotated into the dst bitmap
	// (0,0)        (w,0)
	//   +------------+
	//   |0          1|
	//   |2          3|
	//   +------------+
	// (0,h)        (w,h)

	if (srcClipRect == NULL) {
		srcCorner[0] = GPoint(0,0);
		srcCorner[1] = GPoint(src->bounds.size.w,0);
		srcCorner[2] = GPoint(0,src->bounds.size.h);
		srcCorner[3] = GPoint(src->bounds.size.w,src->bounds.size.h);
	} else {
		srcCorner[0] = GPoint(srcClipRect->origin.x, srcClipRect->origin.y);
		srcCorner[1] = GPoint(srcClipRect->origin.x + srcClipRect->size.w, srcClipRect->origin.y);
		srcCorner[2] = GPoint(srcClipRect->origin.x, srcClipRect->origin.y + srcClipRect->size.h);
		srcCorner[3] = GPoint(srcClipRect->origin.x + srcClipRect->size.w, srcClipRect->origin.y + srcClipRect->size.h);
	}

	cosphi = _COS(a);
	sinphi = _SIN(a);
	for (i=0; i<4; i++) {
		xo = srcCorner[i].x - srcCenter.x;
		yo = srcCorner[i].y - srcCenter.y;
		dstCorner[i].x = TRIG_NORM(xo*cosphi - yo*sinphi) + srcCenter.x + dstOffset.x;
		dstCorner[i].y = TRIG_NORM(xo*sinphi + yo*cosphi) + srcCenter.y + dstOffset.y;
		// Keep min & max for clipping
		xmin = MIN(xmin,dstCorner[i].x);
		ymin = MIN(ymin,dstCorner[i].y);
		xmax = MAX(xmax,dstCorner[i].x);
		ymax = MAX(ymax,dstCorner[i].y);
	}

	// Take 1 pixel more because of int rounding errors
	xmin--; ymin--; xmax++; ymax++;
	// stay in the limits of the dst bitmap
	if (xmin < 0) xmin = 0;
	if (ymin < 0) ymin = 0;
	if (xmax > dst->bounds.size.w) xmax = dst->bounds.size.w;
	if (ymax > dst->bounds.size.h) ymax = dst->bounds.size.h;

	// Only loop into rotated coordinates of the dst bitmap into the (xmin,ymin)/(xmax,ymax) rect
	// and get for each dst pixel the corresponding src pixel
	sinphi = -sinphi;
	xo = xmin - dstOffset.x - srcCenter.x;
	xc = xo*cosphi;
	xs = xo*sinphi;
	yo = ymin - dstOffset.y - srcCenter.y;
	yc = yo*cosphi;
	ys = yo*sinphi;
	for (y=ymin; y<=ymax; y++) {
		xx = xc - ys;
		yy = xs + yc;
		for (x=xmin; x<=xmax; x++) {
			c = bmpGetPixel(src, TRIG_NORM(xx) + srcCenter.x, TRIG_NORM(yy) + srcCenter.y);
			if (c >= 0) bmpPutPixel(dst, x, y, c);
			xx += cosphi;
			yy += sinphi;
		}
		yc += cosphi;
		ys += sinphi;
	}
}

