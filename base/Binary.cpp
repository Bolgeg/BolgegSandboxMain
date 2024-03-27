namespace binaryTransference
{
	const static bool read=false;
	const static bool write=true;
	
	template <class, template <class> class>
	struct is_instance : public std::false_type {};

	template <class T, template <class> class U>
	struct is_instance<U<T>, U> : public std::true_type {};
	
	template <class T,bool readWrite>
	void transfer(T& data,vector<unsigned char>& content,size_t& cursor)
	{
		if constexpr(is_instance<T,vector>{})
		{
			if(readWrite==write)
			{
				uint64_t size=data.size();
				transfer<uint64_t,write>(size,content,cursor);
			}
			else
			{
				uint64_t size;
				transfer<uint64_t,read>(size,content,cursor);
				data.resize(size);
			}
			for(int n=0;n<data.size();n++)
			{
				transfer<typename T::value_type,readWrite>(data[n],content,cursor);
			}
		}
		else if constexpr(is_instance<T,Vector2Type>{})
		{
			transfer<typename T::value_type,readWrite>(data.x,content,cursor);
			transfer<typename T::value_type,readWrite>(data.y,content,cursor);
		}
		else if constexpr(is_instance<T,Vector3Type>{})
		{
			transfer<typename T::value_type,readWrite>(data.x,content,cursor);
			transfer<typename T::value_type,readWrite>(data.y,content,cursor);
			transfer<typename T::value_type,readWrite>(data.z,content,cursor);
		}
		else if constexpr(is_instance<T,VectorType>{})
		{
			transfer<vector<typename T::value_type>,readWrite>(data.coordinates,content,cursor);
		}
		else
		{
			throw string("Binary transfer not defined for: ")+typeid(T).name();
		}
	}
	
	template <>
	void transfer<uint8_t,write>(uint8_t& data,vector<unsigned char>& content,size_t& cursor)
	{
		content.push_back(data);
	}
	template <>
	void transfer<uint8_t,read>(uint8_t& data,vector<unsigned char>& content,size_t& cursor)
	{
		data=content.at(cursor);
		cursor++;
	}
	template <>
	void transfer<uint16_t,write>(uint16_t& data,vector<unsigned char>& content,size_t& cursor)
	{
		content.push_back(data&0xff);
		content.push_back(data>>8);
	}
	template <>
	void transfer<uint16_t,read>(uint16_t& data,vector<unsigned char>& content,size_t& cursor)
	{
		data=content.at(cursor);
		data|=content.at(cursor+1)<<8;
		cursor+=2;
	}
	template <>
	void transfer<uint32_t,write>(uint32_t& data,vector<unsigned char>& content,size_t& cursor)
	{
		content.push_back(data&0xff);
		content.push_back((data>>8)&0xff);
		content.push_back((data>>16)&0xff);
		content.push_back(data>>24);
	}
	template <>
	void transfer<uint32_t,read>(uint32_t& data,vector<unsigned char>& content,size_t& cursor)
	{
		data=content.at(cursor);
		data|=content.at(cursor+1)<<8;
		data|=content.at(cursor+2)<<16;
		data|=content.at(cursor+3)<<24;
		cursor+=4;
	}
	template <>
	void transfer<uint64_t,write>(uint64_t& data,vector<unsigned char>& content,size_t& cursor)
	{
		content.push_back(data&0xff);
		content.push_back((data>>8)&0xff);
		content.push_back((data>>16)&0xff);
		content.push_back((data>>24)&0xff);
		content.push_back((data>>32)&0xff);
		content.push_back((data>>40)&0xff);
		content.push_back((data>>48)&0xff);
		content.push_back(data>>56);
	}
	template <>
	void transfer<uint64_t,read>(uint64_t& data,vector<unsigned char>& content,size_t& cursor)
	{
		data=content.at(cursor);
		data|=content.at(cursor+1)<<8;
		data|=content.at(cursor+2)<<16;
		data|=content.at(cursor+3)<<24;
		data|=((uint64_t)content.at(cursor+4))<<32;
		data|=((uint64_t)content.at(cursor+5))<<40;
		data|=((uint64_t)content.at(cursor+6))<<48;
		data|=((uint64_t)content.at(cursor+7))<<56;
		cursor+=8;
	}
	template <>
	void transfer<float,write>(float& data,vector<unsigned char>& content,size_t& cursor)
	{
		unsigned char*p=(unsigned char*)&data;
		for(int b=0;b<4;b++)
		{
			transfer<uint8_t,write>(p[b],content,cursor);
		}
	}
	template <>
	void transfer<float,read>(float& data,vector<unsigned char>& content,size_t& cursor)
	{
		unsigned char*p=(unsigned char*)&data;
		for(int b=0;b<4;b++)
		{
			transfer<uint8_t,read>(p[b],content,cursor);
		}
	}
	template <>
	void transfer<double,write>(double& data,vector<unsigned char>& content,size_t& cursor)
	{
		unsigned char*p=(unsigned char*)&data;
		for(int b=0;b<8;b++)
		{
			transfer<uint8_t,write>(p[b],content,cursor);
		}
	}
	template <>
	void transfer<double,read>(double& data,vector<unsigned char>& content,size_t& cursor)
	{
		unsigned char*p=(unsigned char*)&data;
		for(int b=0;b<8;b++)
		{
			transfer<uint8_t,read>(p[b],content,cursor);
		}
	}
	template <>
	void transfer<string,write>(string& data,vector<unsigned char>& content,size_t& cursor)
	{
		uint64_t size=data.size();
		transfer<uint64_t,write>(size,content,cursor);
		for(int n=0;n<data.size();n++)
		{
			transfer<uint8_t,write>((uint8_t&)data[n],content,cursor);
		}
	}
	template <>
	void transfer<string,read>(string& data,vector<unsigned char>& content,size_t& cursor)
	{
		uint64_t size;
		transfer<uint64_t,read>(size,content,cursor);
		data.resize(size);
		for(int n=0;n<data.size();n++)
		{
			transfer<uint8_t,read>((uint8_t&)data[n],content,cursor);
		}
	}
	
	template <>
	void transfer<int8_t,write>(int8_t& data,vector<unsigned char>& content,size_t& cursor)
	{
		transfer<uint8_t,write>((uint8_t&)data,content,cursor);
	}
	template <>
	void transfer<int8_t,read>(int8_t& data,vector<unsigned char>& content,size_t& cursor)
	{
		transfer<uint8_t,read>((uint8_t&)data,content,cursor);
	}
	template <>
	void transfer<int16_t,write>(int16_t& data,vector<unsigned char>& content,size_t& cursor)
	{
		transfer<uint16_t,write>((uint16_t&)data,content,cursor);
	}
	template <>
	void transfer<int16_t,read>(int16_t& data,vector<unsigned char>& content,size_t& cursor)
	{
		transfer<uint16_t,read>((uint16_t&)data,content,cursor);
	}
	template <>
	void transfer<int32_t,write>(int32_t& data,vector<unsigned char>& content,size_t& cursor)
	{
		transfer<uint32_t,write>((uint32_t&)data,content,cursor);
	}
	template <>
	void transfer<int32_t,read>(int32_t& data,vector<unsigned char>& content,size_t& cursor)
	{
		transfer<uint32_t,read>((uint32_t&)data,content,cursor);
	}
	template <>
	void transfer<int64_t,write>(int64_t& data,vector<unsigned char>& content,size_t& cursor)
	{
		transfer<uint64_t,write>((uint64_t&)data,content,cursor);
	}
	template <>
	void transfer<int64_t,read>(int64_t& data,vector<unsigned char>& content,size_t& cursor)
	{
		transfer<uint64_t,read>((uint64_t&)data,content,cursor);
	}
}

class Binary
{
	public:
	vector<unsigned char> content;
	size_t cursor=0;
	static size_t getSizeOfFile(const string& filePath)
	{
		ifstream in(filePath,ifstream::ate | ifstream::binary);
		return in.tellg();
	}
	void saveToFile(const string& filePath)
	{
		ofstream outputFile;
		outputFile.open(filePath,ios::binary);
		
		Binary start;
		size_t _size=size();
		start.transfer<uint64_t,binaryTransference::write>(_size);
		outputFile.write(reinterpret_cast<char*>(start.data()),start.size());
		
		outputFile.write(reinterpret_cast<char*>(data()),size());
		
		outputFile.close();
	}
	void loadFromFile(const string& filePath)
	{
		ifstream inputFile;
		inputFile.open(filePath,ios::binary);
		if(!inputFile.is_open()) throw string("Error opening file at: ")+filePath;
		
		Binary start;
		start.resize(sizeof(uint64_t));
		inputFile.read(reinterpret_cast<char*>(start.data()),sizeof(uint64_t));
		start.cursor=0;
		size_t bytes;
		start.transfer<uint64_t,binaryTransference::read>(bytes);
		
		resize(bytes);
		inputFile.read(reinterpret_cast<char*>(data()),bytes);
		
		inputFile.close();
	}
	void saveToFileRaw(const string& filePath)
	{
		ofstream outputFile;
		outputFile.open(filePath,ios::binary);
		
		outputFile.write(reinterpret_cast<char*>(data()),size());
		
		outputFile.close();
	}
	void loadFromFileRaw(const string& filePath)
	{
		size_t bytes=getSizeOfFile(filePath);
		
		ifstream inputFile;
		inputFile.open(filePath,ios::binary);
		if(!inputFile.is_open()) throw string("Error opening file at: ")+filePath;
		
		resize(bytes);
		
		inputFile.read(reinterpret_cast<char*>(data()),bytes);
		
		inputFile.close();
	}
	size_t size()
	{
		return content.size();
	}
	void resize(size_t newSize)
	{
		content.resize(newSize);
	}
	size_t capacity()
	{
		return content.capacity();
	}
	bool empty()
	{
		return content.empty();
	}
	void reserve(size_t n)
	{
		content.reserve(n);
	}
	void shrink_to_fit()
	{
		content.shrink_to_fit();
	}
	unsigned char& operator [](size_t n)
	{
		return content[n];
	}
	unsigned char& at(size_t n)
	{
		return content.at(n);
	}
	unsigned char& front()
	{
		return content.front();
	}
	unsigned char& back()
	{
		return content.back();
	}
	unsigned char*data()
	{
		return content.data();
	}
	template <class T,bool readWrite>
	void transfer(T& data)
	{
		binaryTransference::transfer<T,readWrite>(data,content,cursor);
	}
	template <class T>
	void write(T data)
	{
		transfer<T,binaryTransference::write>(data);
	}
	template <class T>
	T read()
	{
		T data;
		transfer<T,binaryTransference::read>(data);
		return data;
	}
};

