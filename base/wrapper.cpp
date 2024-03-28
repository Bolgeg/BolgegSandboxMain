#define KEY_F1 128+1
#define KEY_F2 128+2
#define KEY_F3 128+3
#define KEY_F4 128+4
#define KEY_F5 128+5
#define KEY_F6 128+6
#define KEY_F7 128+7
#define KEY_F8 128+8
#define KEY_F9 128+9
#define KEY_F10 128+10
#define KEY_F11 128+11
#define KEY_F12 128+12

#define KEY_BACKSPACE 160
#define KEY_CAPSLOCK 161
#define KEY_DELETE 162

#define KEY_UP 163
#define KEY_DOWN 164
#define KEY_LEFT 165
#define KEY_RIGHT 166

#define KEY_ESCAPE 167

#define KEY_LALT 168
#define KEY_RALT 169
#define KEY_LCTRL 170
#define KEY_RCTRL 171
#define KEY_LSHIFT 172
#define KEY_RSHIFT 173

#define KEY_ENTER 174
#define KEY_PERIOD 175
#define KEY_SPACE 176
#define KEY_TAB 177

#define KEY_LBUTTON 200
#define KEY_MBUTTON 201
#define KEY_RBUTTON 202

class Input
{
	public:
	vector<bool> key=vector<bool>(256,false);
	vector<bool> keyOld=vector<bool>(256,false);
	Pos mouse=Pos(0,0);
	Pos mouseOld=Pos(0,0);
};

void (*CURRENT_AUDIO_CALLBACK_FUNCTION)(float*,int)=nullptr;

void audioCallbackWrapper(void*userdata,Uint8*stream,int len)
{
	if(CURRENT_AUDIO_CALLBACK_FUNCTION!=nullptr) (*CURRENT_AUDIO_CALLBACK_FUNCTION)((float*)stream,len/8);
}

void (*CURRENT_INPUT_AUDIO_CALLBACK_FUNCTION)(float*,int)=nullptr;

void inputAudioCallbackWrapper(void*userdata,Uint8*stream,int len)
{
	if(CURRENT_INPUT_AUDIO_CALLBACK_FUNCTION!=nullptr) (*CURRENT_INPUT_AUDIO_CALLBACK_FUNCTION)((float*)stream,len/4);
}

class WindowJoystickController
{
	public:
	SDL_Joystick*sdlJoystick;
	vector<bool> buttons;
	vector<double> axes;
	vector<Pos> hats;
	void initialize(SDL_Joystick*_sdlJoystick,int numButtons,int numAxes,int numHats)
	{
		sdlJoystick=_sdlJoystick;
		buttons=vector<bool>(numButtons,false);
		axes=vector<double>(numAxes,0);
		hats=vector<Pos>(numHats,Pos(0,0));
	}
	vector<string> stateToStringVector()
	{
		vector<string> v(3);
		
		for(int i=0;i<buttons.size();i++)
		{
			v[0]+=to_string(buttons[i] ? 1 : 0)+" ";
		}
		
		for(int i=0;i<axes.size();i++)
		{
			v[1]+=to_string(axes[i])+" ";
		}
		
		for(int i=0;i<hats.size();i++)
		{
			v[2]+=to_string(hats[i].x)+" "+to_string(hats[i].y)+" ";
		}
		
		return v;
	}
	string stateToString()
	{
		vector<string> v=stateToStringVector();
		string str;
		for(int line=0;line<v.size();line++)
		{
			str+=v[line]+"\n";
		}
		return str;
	}
};

class Window
{
	public:
	bool created=false;
	Pos maximumResolution=Pos(0,0);
	SDL_Window*window=nullptr;
	SDL_Renderer*renderer=nullptr;
	SDL_Texture*screenTexture=nullptr;
	Image screen;
	Input input;
	map<SDL_Keycode,int> keyMap;
	unsigned int lastFrameTime=0;
	double SF=0;
	bool audioInitialized=false;
	bool audioDeviceOpened=false;
	SDL_AudioDeviceID audioDeviceId;
	bool inputAudioDeviceOpened=false;
	SDL_AudioDeviceID inputAudioDeviceId;
	
	bool joystickControllersInitialized=false;
	vector<WindowJoystickController> joystickControllers;
	
	void create(const string& windowTitle="Program")
	{
		if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)!=0)
		{
			throw string("SDL could not be initialized");
		}
		
		window=SDL_CreateWindow(windowTitle.c_str(),SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,640,480,
			SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED | SDL_WINDOW_OPENGL);
		if(window==nullptr)
		{
			throw string("The window could not be created");
		}
		renderer=SDL_CreateRenderer(window,-1,SDL_RENDERER_PRESENTVSYNC);
		if(renderer==nullptr)
		{
			throw string("The window renderer could not be created");
		}
		
		maximumResolution=getScreenResolution();
		
		screenTexture=SDL_CreateTexture(renderer,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,maximumResolution.x,maximumResolution.y);
		
		screen=Image(maximumResolution);
		
		initializeKeyMap();
		
		created=true;
	}
	void destroy()
	{
		closeAudio();
		
		if(joystickControllersInitialized)
		{
			for(int i=0;i<joystickControllers.size();i++)
			{
				SDL_JoystickClose(joystickControllers[i].sdlJoystick);
			}
			joystickControllers.resize(0);
		}
		
		SDL_Quit();
	}
	void setWindowIcon(Image& image)
	{
		SDL_Surface*icon=SDL_CreateRGBSurfaceFrom(image.data(),image.size().x,image.size().y,32,4*image.size().x,0xff0000,0x00ff00,0x0000ff,0);
		SDL_SetWindowIcon(window,icon);
	}
	void initializeJoystickControllers()
	{
		if(!joystickControllersInitialized)
		{
			SDL_Init(SDL_INIT_JOYSTICK);
			joystickControllersInitialized=true;
		}
	}
	void setFullscreen(bool fullscreen)
	{
		SDL_SetWindowFullscreen(window,fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
	}
	Pos getScreenResolution()
	{
		SDL_DisplayMode dm;
		if(SDL_GetDesktopDisplayMode(0,&dm))
		{
			throw string("Could not get the size of the display");
		}
		return Pos(dm.w,dm.h);
	}
	Pos getWindowSize()
	{
		Pos p;
		SDL_GetWindowSize(window,&p.x,&p.y);
		return p;
	}
	unsigned int getMilliseconds()
	{
		return SDL_GetTicks();
	}
	void drawScreen()
	{
		SDL_UpdateTexture(screenTexture,NULL,screen.data(),screen.size().x*sizeof(int));
		SDL_RenderClear(renderer);
		
		Pos s=getWindowSize();
		SDL_Rect rect;
		rect.x=0;
		rect.y=0;
		rect.w=s.x;
		rect.h=s.y;
		
		SDL_RenderCopy(renderer,screenTexture,&rect,&rect);
		SDL_RenderPresent(renderer);
	}
	bool pollEvents()
	{
		bool continueRunning=true;
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			if(event.type==SDL_QUIT)
			{
				continueRunning=false;
				break;
			}
		}
		return continueRunning;
	}
	void updateTime()
	{
		unsigned int timeNow=getMilliseconds();
		unsigned int time=timeNow-lastFrameTime;
		lastFrameTime=timeNow;
		SF=double(time)/1000;
	}
	bool pollEventsAndUpdateInputAndTime()
	{
		bool continueRunning=pollEvents();
		if(!continueRunning) return false;
		updateInput();
		updateTime();
		return continueRunning;
	}
	void initializeKeyMap()
	{
		keyMap[SDLK_1]='1';
		keyMap[SDLK_2]='2';
		keyMap[SDLK_3]='3';
		keyMap[SDLK_4]='4';
		keyMap[SDLK_5]='5';
		keyMap[SDLK_6]='6';
		keyMap[SDLK_7]='7';
		keyMap[SDLK_8]='8';
		keyMap[SDLK_9]='9';
		keyMap[SDLK_0]='0';
		
		keyMap[SDLK_q]='Q';
		keyMap[SDLK_w]='W';
		keyMap[SDLK_e]='E';
		keyMap[SDLK_r]='R';
		keyMap[SDLK_t]='T';
		keyMap[SDLK_y]='Y';
		keyMap[SDLK_u]='U';
		keyMap[SDLK_i]='I';
		keyMap[SDLK_o]='O';
		keyMap[SDLK_p]='P';
		keyMap[SDLK_a]='A';
		keyMap[SDLK_s]='S';
		keyMap[SDLK_d]='D';
		keyMap[SDLK_f]='F';
		keyMap[SDLK_g]='G';
		keyMap[SDLK_h]='H';
		keyMap[SDLK_j]='J';
		keyMap[SDLK_k]='K';
		keyMap[SDLK_l]='L';
		keyMap[SDLK_z]='Z';
		keyMap[SDLK_x]='X';
		keyMap[SDLK_c]='C';
		keyMap[SDLK_v]='V';
		keyMap[SDLK_b]='B';
		keyMap[SDLK_n]='N';
		keyMap[SDLK_m]='M';
		
		keyMap[SDLK_F1]=KEY_F1;
		keyMap[SDLK_F2]=KEY_F2;
		keyMap[SDLK_F3]=KEY_F3;
		keyMap[SDLK_F4]=KEY_F4;
		keyMap[SDLK_F5]=KEY_F5;
		keyMap[SDLK_F6]=KEY_F6;
		keyMap[SDLK_F7]=KEY_F7;
		keyMap[SDLK_F8]=KEY_F8;
		keyMap[SDLK_F9]=KEY_F9;
		keyMap[SDLK_F10]=KEY_F10;
		keyMap[SDLK_F11]=KEY_F11;
		keyMap[SDLK_F12]=KEY_F12;
		
		keyMap[SDLK_BACKSPACE]=KEY_BACKSPACE;
		keyMap[SDLK_CAPSLOCK]=KEY_CAPSLOCK;
		keyMap[SDLK_DELETE]=KEY_DELETE;
		
		keyMap[SDLK_UP]=KEY_UP;
		keyMap[SDLK_DOWN]=KEY_DOWN;
		keyMap[SDLK_LEFT]=KEY_LEFT;
		keyMap[SDLK_RIGHT]=KEY_RIGHT;
		
		keyMap[SDLK_ESCAPE]=KEY_ESCAPE;
		
		keyMap[SDLK_LALT]=KEY_LALT;
		keyMap[SDLK_RALT]=KEY_RALT;
		keyMap[SDLK_LCTRL]=KEY_LCTRL;
		keyMap[SDLK_RCTRL]=KEY_RCTRL;
		keyMap[SDLK_LSHIFT]=KEY_LSHIFT;
		keyMap[SDLK_RSHIFT]=KEY_RSHIFT;
		
		keyMap[SDLK_RETURN]=KEY_ENTER;
		keyMap[SDLK_PERIOD]=KEY_PERIOD;
		keyMap[SDLK_SPACE]=KEY_SPACE;
		keyMap[SDLK_TAB]=KEY_TAB;
	}
	void updateInput()
	{
		input.keyOld=input.key;
		for(int k=0;k<input.key.size();k++)
		{
			input.key[k]=false;
		}
		int numKeys;
		const Uint8*keys=SDL_GetKeyboardState(&numKeys);
		for(int k=0;k<numKeys;k++)
		{
			if(keys[k])
			{
				SDL_Keycode keycode=SDL_GetKeyFromScancode((SDL_Scancode)k);
				try
				{
					int keyNumber=keyMap.at(keycode);
					if(keyNumber>=0 && keyNumber<input.key.size()) input.key[keyNumber]=true;
				}
				catch(...){}
			}
		}
		input.mouseOld=input.mouse;
		Uint32 buttons=SDL_GetMouseState(&input.mouse.x,&input.mouse.y);
		if(buttons&SDL_BUTTON_LMASK) input.key[KEY_LBUTTON]=true;
		if(buttons&SDL_BUTTON_MMASK) input.key[KEY_MBUTTON]=true;
		if(buttons&SDL_BUTTON_RMASK) input.key[KEY_RBUTTON]=true;
		
		if(joystickControllersInitialized)
		{
			updateJoystickControllers();
		}
	}
	void updateJoystickControllers()
	{
		if(joystickControllersInitialized)
		{
			int numControllers=SDL_NumJoysticks();
			if(numControllers!=joystickControllers.size())
			{
				for(int i=0;i<joystickControllers.size();i++)
				{
					SDL_JoystickClose(joystickControllers[i].sdlJoystick);
				}
				joystickControllers.resize(0);
				joystickControllers.resize(numControllers);
				
				for(int i=0;i<joystickControllers.size();i++)
				{
					SDL_Joystick*sdlJoystick=SDL_JoystickOpen(i);
					joystickControllers[i].initialize(sdlJoystick,SDL_JoystickNumButtons(sdlJoystick),SDL_JoystickNumAxes(sdlJoystick),SDL_JoystickNumHats(sdlJoystick));
				}
			}
			
			for(int i=0;i<joystickControllers.size();i++)
			{
				for(int j=0;j<joystickControllers[i].buttons.size();j++)
				{
					joystickControllers[i].buttons[j]=SDL_JoystickGetButton(joystickControllers[i].sdlJoystick,j);
				}
				for(int j=0;j<joystickControllers[i].axes.size();j++)
				{
					joystickControllers[i].axes[j]=double(SDL_JoystickGetAxis(joystickControllers[i].sdlJoystick,j))/32768;
				}
				for(int j=0;j<joystickControllers[i].hats.size();j++)
				{
					int value=SDL_JoystickGetHat(joystickControllers[i].sdlJoystick,j);
					Pos position(0,0);
					
					if(value&SDL_HAT_UP) position.y--;
					if(value&SDL_HAT_DOWN) position.y++;
					if(value&SDL_HAT_LEFT) position.x--;
					if(value&SDL_HAT_RIGHT) position.x++;
					
					joystickControllers[i].hats[j]=position;
				}
			}
		}
	}
	void setMousePosition(Pos position)
	{
		SDL_WarpMouseInWindow(window,position.x,position.y);
	}
	void setShowCursor(bool show)
	{
		if(show) SDL_ShowCursor(SDL_ENABLE);
		else SDL_ShowCursor(SDL_DISABLE);
	}
	void initializeAudio()
	{
		SDL_Init(SDL_INIT_AUDIO);
		audioInitialized=true;
	}
	void pauseAudio()
	{
		SDL_PauseAudioDevice(audioDeviceId,1);
	}
	void unpauseAudio()
	{
		SDL_PauseAudioDevice(audioDeviceId,0);
	}
	void openAudio(void (*callbackFunction)(float*,int),int samples)
	{
		if(!audioInitialized)
		{
			initializeAudio();
		}
		
		if(audioDeviceOpened) throw string("The audio device is already opened");
		
		SDL_AudioSpec want,have;
		SDL_memset(&want,0,sizeof(want));
		want.freq=AUDIO_SAMPLE_RATE;
		want.format=AUDIO_F32SYS;
		want.channels=2;
		want.samples=samples;
		want.callback=audioCallbackWrapper;
		CURRENT_AUDIO_CALLBACK_FUNCTION=callbackFunction;
		audioDeviceId=SDL_OpenAudioDevice(NULL,0,&want,&have,0);
		if(audioDeviceId==0)
		{
			throw string("Could not open the audio device: ")+SDL_GetError();
		}
		
		unpauseAudio();
		
		audioDeviceOpened=true;
	}
	void closeAudio()
	{
		if(audioDeviceOpened)
		{
			pauseAudio();
			SDL_CloseAudioDevice(audioDeviceId);
			CURRENT_AUDIO_CALLBACK_FUNCTION=nullptr;
			audioDeviceOpened=false;
		}
		if(inputAudioDeviceOpened)
		{
			pauseInputAudio();
			SDL_CloseAudioDevice(inputAudioDeviceId);
			CURRENT_INPUT_AUDIO_CALLBACK_FUNCTION=nullptr;
			inputAudioDeviceOpened=false;
		}
	}
	void pauseInputAudio()
	{
		SDL_PauseAudioDevice(inputAudioDeviceId,1);
	}
	void unpauseInputAudio()
	{
		SDL_PauseAudioDevice(inputAudioDeviceId,0);
	}
	void openInputAudio(void (*callbackFunction)(float*,int),int samples)
	{
		if(!audioInitialized)
		{
			initializeAudio();
		}
		
		if(inputAudioDeviceOpened) throw string("The input audio device is already opened");
		
		SDL_AudioSpec want,have;
		SDL_memset(&want,0,sizeof(want));
		want.freq=AUDIO_SAMPLE_RATE;
		want.format=AUDIO_F32SYS;
		want.channels=1;
		want.samples=samples;
		want.callback=inputAudioCallbackWrapper;
		CURRENT_INPUT_AUDIO_CALLBACK_FUNCTION=callbackFunction;
		inputAudioDeviceId=SDL_OpenAudioDevice(NULL,1,&want,&have,0);
		if(inputAudioDeviceId==0)
		{
			throw string("Could not open the input audio device: ")+SDL_GetError();
		}
		
		unpauseInputAudio();
		
		inputAudioDeviceOpened=true;
	}
};

