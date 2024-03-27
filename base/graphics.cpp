class DrawingArea
{
	public:
	Pos min=Pos(0,0);
	Pos max=Pos(0,0);
	bool isDefault=true;
	DrawingArea(){}
	DrawingArea(Pos _min,Pos _max)
	{
		min=_min;
		max=_max;
		isDefault=false;
	}
	bool contains(Pos p) const
	{
		return p.within(min,max);
	}
	bool isEmpty() const
	{
		return min.x>max.x || min.y>max.y;
	}
	static void intersection1D(int amin,int amax,int bmin,int bmax,int& rmin,int& rmax)
	{
		if(amax>=bmin && bmax>=amin)
		{
			if(amin>=bmin)
			{
				if(bmax>=amax)
				{
					rmin=amin;
					rmax=amax;
				}
				else
				{
					rmin=amin;
					rmax=bmax;
				}
			}
			else
			{
				if(amax>=bmax)
				{
					rmin=bmin;
					rmax=bmax;
				}
				else
				{
					rmin=bmin;
					rmax=amax;
				}
			}
		}
		else
		{
			rmin=0;
			rmax=-1;
		}
	}
	static DrawingArea intersection(DrawingArea a,DrawingArea b)
	{
		DrawingArea area(Pos(0,0),Pos(0,0));
		intersection1D(a.min.x,a.max.x,b.min.x,b.max.x,area.min.x,area.max.x);
		intersection1D(a.min.y,a.max.y,b.min.y,b.max.y,area.min.y,area.max.y);
		return area;
	}
}DEFAULT_DRAWING_AREA;

class Image
{
	private:
	Pos internalSize=Pos(0,0);
	vector<int> internalPixels;
	public:
	Pos size() const
	{
		return internalSize;
	}
	int*data()
	{
		return internalPixels.data();
	}
	Image(){}
	explicit Image(Pos _size)
	{
		if(_size.x<0) _size.x=0;
		if(_size.y<0) _size.y=0;
		internalSize=_size;
		internalPixels=vector<int>(internalSize.x*internalSize.y);
	}
	explicit Image(const string& filePath)
	{
		load(filePath);
	}
	void load(const string& filePath)
	{
		Binary binary;
		binary.loadFromFileRaw(filePath);
		
		binary.cursor=18;
		internalSize.x=binary.read<uint32_t>();
		internalSize.y=binary.read<uint32_t>();
		internalPixels=vector<int>(internalSize.x*internalSize.y);
		
		binary.cursor=54;
		int padding=(4-((internalSize.x*3)%4))%4;
		for(int y=internalSize.y-1;y>=0;y--)
		{
			for(int x=0;x<internalSize.x;x++)
			{
				int color=binary.read<uint8_t>();
				color|=binary.read<uint8_t>()<<8;
				color|=binary.read<uint8_t>()<<16;
				internalPixels[y*internalSize.x+x]=color;
			}
			binary.cursor+=padding;
		}
	}
	void save(const string& filePath)
	{
		Binary binary;
		
		binary.write<uint8_t>('B');
		binary.write<uint8_t>('M');
		
		int sizePosition=binary.size();
		
		binary.write<uint32_t>(0);
		binary.write<uint32_t>(0);
		binary.write<uint32_t>(0x36);
		binary.write<uint32_t>(40);
		binary.write<uint32_t>(internalSize.x);
		binary.write<uint32_t>(internalSize.y);
		
		binary.write<uint16_t>(1);
		binary.write<uint16_t>(24);
		
		binary.write<uint32_t>(0);
		binary.write<uint32_t>(0);
		binary.write<uint32_t>(0);
		binary.write<uint32_t>(0);
		binary.write<uint32_t>(0);
		binary.write<uint32_t>(0);
		
		int padding=(4-((internalSize.x*3)%4))%4;
		for(int y=internalSize.y-1;y>=0;y--)
		{
			for(int x=0;x<internalSize.x;x++)
			{
				int color=internalPixels[y*internalSize.x+x];
				binary.write<uint8_t>(color&0xff);
				binary.write<uint8_t>((color>>8)&0xff);
				binary.write<uint8_t>((color>>16)&0xff);
			}
			for(int n=0;n<padding;n++)
			{
				binary.write<uint8_t>(0);
			}
		}
		
		binary.content[sizePosition]=binary.size()&0xff;
		binary.content[sizePosition+1]=(binary.size()>>8)&0xff;
		binary.content[sizePosition+2]=(binary.size()>>16)&0xff;
		binary.content[sizePosition+3]=(binary.size()>>24)&0xff;
		
		binary.saveToFileRaw(filePath);
	}
	void clear(int color)
	{
		for(size_t p=0;p<internalPixels.size();p++)
		{
			internalPixels[p]=color;
		}
	}
	int getpixel(Pos p) const
	{
		if(p.x<0 || p.y<0 || p.x>=internalSize.x || p.y>=internalSize.y) return -1;
		else return internalPixels[p.y*internalSize.x+p.x];
	}
	int _getpixel(Pos p) const
	{
		return internalPixels[p.y*internalSize.x+p.x];
	}
	void putpixel(Pos p,int color)
	{
		if(p.x>=0 && p.y>=0 && p.x<internalSize.x && p.y<internalSize.y)
		{
			internalPixels[p.y*internalSize.x+p.x]=color;
		}
	}
	void _putpixel(Pos p,int color)
	{
		internalPixels[p.y*internalSize.x+p.x]=color;
	}
	DrawingArea getDrawingArea() const
	{
		return DrawingArea(Pos(0,0),Pos(internalSize.x-1,internalSize.y-1));
	}
	void adjustDrawingArea(DrawingArea& drawingArea) const
	{
		DrawingArea full=getDrawingArea();
		if(drawingArea.isDefault) drawingArea=full;
		else drawingArea=DrawingArea::intersection(drawingArea,full);
	}
	void putpixel(Pos p,int color,DrawingArea drawingArea)
	{
		adjustDrawingArea(drawingArea);
		if(drawingArea.contains(p)) internalPixels[p.y*internalSize.x+p.x]=color;
	}
	void rectfill(Pos min,Pos max,int color,DrawingArea drawingArea=DEFAULT_DRAWING_AREA)
	{
		adjustDrawingArea(drawingArea);
		DrawingArea area=DrawingArea::intersection(drawingArea,DrawingArea(min,max));
		if(!area.isEmpty())
		{
			for(int y=area.min.y;y<=area.max.y;y++)
			{
				for(int x=area.min.x;x<=area.max.x;x++)
				{
					internalPixels[y*internalSize.x+x]=color;
				}
			}
		}
	}
	void hline(Pos p,int x2,int color,DrawingArea drawingArea=DEFAULT_DRAWING_AREA)
	{
		adjustDrawingArea(drawingArea);
		if(x2<p.x) swap(p.x,x2);
		DrawingArea area=DrawingArea::intersection(drawingArea,DrawingArea(p,Pos(x2,p.y)));
		if(!area.isEmpty())
		{
			int y=area.min.y;
			for(int x=area.min.x;x<=area.max.x;x++)
			{
				internalPixels[y*internalSize.x+x]=color;
			}
		}
	}
	void vline(Pos p,int y2,int color,DrawingArea drawingArea=DEFAULT_DRAWING_AREA)
	{
		adjustDrawingArea(drawingArea);
		if(y2<p.y) swap(p.y,y2);
		DrawingArea area=DrawingArea::intersection(drawingArea,DrawingArea(p,Pos(p.x,y2)));
		if(!area.isEmpty())
		{
			int x=area.min.x;
			for(int y=area.min.y;y<=area.max.y;y++)
			{
				internalPixels[y*internalSize.x+x]=color;
			}
		}
	}
	void line(Pos a,Pos b,int color,DrawingArea drawingArea=DEFAULT_DRAWING_AREA)
	{
		if(a.y==b.y) hline(a,b.x,color,drawingArea);
		else if(a.x==b.x) vline(a,b.y,color,drawingArea);
		else
		{
			adjustDrawingArea(drawingArea);
			int d=ceil((b-a).d<double>())+1;
			for(int n=0;n<=d;n++)
			{
				Pos position=Pos(lerp(a.x,b.x,double(n)/d),lerp(a.y,b.y,double(n)/d));
				if(drawingArea.contains(position)) internalPixels[position.y*internalSize.x+position.x]=color;
			}
		}
	}
	void rect(Pos min,Pos max,int color,DrawingArea drawingArea=DEFAULT_DRAWING_AREA)
	{
		hline(min,max.x,color,drawingArea);
		hline(Pos(min.x,max.y),max.x,color,drawingArea);
		vline(min,max.y,color,drawingArea);
		vline(Pos(max.x,min.y),max.y,color,drawingArea);
	}
	void circlefill(Vector2 center,float radius,int color,DrawingArea drawingArea=DEFAULT_DRAWING_AREA)
	{
		adjustDrawingArea(drawingArea);
		Pos min=Vector2(center.x-radius,center.y-radius).floor();
		Pos max=Vector2(center.x+radius,center.y+radius).ceil()-Pos(1,1);
		DrawingArea area=DrawingArea::intersection(drawingArea,DrawingArea(min,max));
		if(!area.isEmpty())
		{
			float radius2=radius*radius;
			for(int y=area.min.y;y<=area.max.y;y++)
			{
				for(int x=area.min.x;x<=area.max.x;x++)
				{
					if(Vector2Type<float>(center.x-x,center.y-y).d2()<=radius2) internalPixels[y*internalSize.x+x]=color;
				}
			}
		}
	}
	DrawingArea blitDrawingArea(const Image& image,Pos destinationPosition,Pos sourcePosition,Pos blitSize,DrawingArea drawingArea=DEFAULT_DRAWING_AREA) const
	{
		adjustDrawingArea(drawingArea);
		DrawingArea areaDestination=DrawingArea::intersection(drawingArea,DrawingArea(destinationPosition,destinationPosition+blitSize-Pos(1,1)));
		DrawingArea areaSource=DrawingArea::intersection(image.getDrawingArea(),DrawingArea(sourcePosition,sourcePosition+blitSize-Pos(1,1)));
		DrawingArea finalArea=DrawingArea::intersection(areaDestination,DrawingArea(areaSource.min+destinationPosition-sourcePosition,areaSource.max+destinationPosition-sourcePosition));
		return finalArea;
	}
	void blit(const Image& image,Pos destinationPosition=Pos(0,0),Pos sourcePosition=Pos(0,0),Pos blitSize=Pos(-1,-1),DrawingArea drawingArea=DEFAULT_DRAWING_AREA)
	{
		if(blitSize==Pos(-1,-1)) blitSize=image.size();
		DrawingArea area=blitDrawingArea(image,destinationPosition,sourcePosition,blitSize,drawingArea);
		if(!area.isEmpty())
		{
			for(int y=area.min.y;y<=area.max.y;y++)
			{
				for(int x=area.min.x;x<=area.max.x;x++)
				{
					internalPixels[y*internalSize.x+x]=image._getpixel(Pos(x,y)-destinationPosition+sourcePosition);
				}
			}
		}
	}
	void maskedBlit(const Image& image,int maskColor,Pos destinationPosition,Pos sourcePosition,Pos blitSize,DrawingArea drawingArea=DEFAULT_DRAWING_AREA)
	{
		DrawingArea area=blitDrawingArea(image,destinationPosition,sourcePosition,blitSize,drawingArea);
		if(!area.isEmpty())
		{
			for(int y=area.min.y;y<=area.max.y;y++)
			{
				for(int x=area.min.x;x<=area.max.x;x++)
				{
					int color=image._getpixel(Pos(x,y)-destinationPosition+sourcePosition);
					if(color!=maskColor) internalPixels[y*internalSize.x+x]=color;
				}
			}
		}
	}
	void maskedBlitColor(const Image& image,int maskColor,int finalColor,Pos destinationPosition,Pos sourcePosition,Pos blitSize,DrawingArea drawingArea=DEFAULT_DRAWING_AREA)
	{
		DrawingArea area=blitDrawingArea(image,destinationPosition,sourcePosition,blitSize,drawingArea);
		if(!area.isEmpty())
		{
			for(int y=area.min.y;y<=area.max.y;y++)
			{
				for(int x=area.min.x;x<=area.max.x;x++)
				{
					int color=image._getpixel(Pos(x,y)-destinationPosition+sourcePosition);
					if(color!=maskColor) internalPixels[y*internalSize.x+x]=finalColor;
				}
			}
		}
	}
	void textprint(const Image& fontImage,Pos position,int color,const string& text,DrawingArea drawingArea=DEFAULT_DRAWING_AREA)
	{
		Pos characterSize=fontImage.size()/Pos(16,16);
		for(int c=0;c<text.size();c++)
		{
			int ch=(unsigned int)(unsigned char)text[c];
			maskedBlitColor(fontImage,0x000000,color,position+Pos(characterSize.x*c,0),characterSize*Pos(ch%16,ch/16),characterSize,drawingArea);
		}
	}
};

