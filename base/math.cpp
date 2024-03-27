double lerp(double a,double b,double t)
{
	return a+(b-a)*t;
}

template <class T=double>
class Vector2Type
{
	public:
	typedef T value_type;
	T x;
	T y;
	Vector2Type(){}
	Vector2Type(T _x,T _y)
	{
		x=_x;
		y=_y;
	}
	Vector2Type(const Vector2Type<int>& v)
	{
		x=v.x;
		y=v.y;
	}
	Vector2Type operator +(Vector2Type b) const
	{
		return Vector2Type(x+b.x,y+b.y);
	}
	Vector2Type operator +=(Vector2Type b)
	{
		return *this=*this+b;
	}
	Vector2Type operator -(Vector2Type b) const
	{
		return Vector2Type(x-b.x,y-b.y);
	}
	Vector2Type operator -=(Vector2Type b)
	{
		return *this=*this-b;
	}
	Vector2Type operator -() const
	{
		return Vector2Type(-x,-y);
	}
	Vector2Type operator *(T b) const
	{
		return Vector2Type(x*b,y*b);
	}
	Vector2Type operator *=(T b)
	{
		return *this=*this*b;
	}
	Vector2Type operator /(T b) const
	{
		return Vector2Type(x/b,y/b);
	}
	Vector2Type operator /=(T b)
	{
		return *this=*this/b;
	}
	Vector2Type operator *(Vector2Type b) const
	{
		return Vector2Type(x*b.x,y*b.y);
	}
	Vector2Type operator *=(Vector2Type b)
	{
		return *this=*this*b;
	}
	Vector2Type operator /(Vector2Type b) const
	{
		return Vector2Type(x/b.x,y/b.y);
	}
	Vector2Type operator /=(Vector2Type b)
	{
		return *this=*this/b;
	}
	bool operator ==(Vector2Type b) const
	{
		return x==b.x && y==b.y;
	}
	bool operator !=(Vector2Type b) const
	{
		return !(*this==b);
	}
	bool within(Vector2Type min,Vector2Type max) const
	{
		return x>=min.x && x<=max.x && y>=min.y && y<=max.y;
	}
	template <class returnType=T>
	returnType d2() const
	{
		return x*x+y*y;
	}
	template <class returnType=T>
	returnType d() const
	{
		return sqrt(d2<returnType>());
	}
	Vector2Type n() const
	{
		T distance=d();
		if(distance==0) return Vector2Type(1,0);
		else return *this/distance;
	}
	Vector2Type<int> toPos() const
	{
		return Vector2Type<int>(x,y);
	}
	Vector2Type<int> floor() const
	{
		return Vector2Type<int>(::floor(x),::floor(y));
	}
	Vector2Type<int> ceil() const
	{
		return Vector2Type<int>(::ceil(x),::ceil(y));
	}
};

using Vector2=Vector2Type<double>;
using Vector2f=Vector2Type<float>;
using Pos=Vector2Type<int>;

template <class T=double>
class Vector3Type
{
	public:
	typedef T value_type;
	T x;
	T y;
	T z;
	Vector3Type(){}
	Vector3Type(T _x,T _y,T _z)
	{
		x=_x;
		y=_y;
		z=_z;
	}
	Vector3Type(const Vector3Type<int>& v)
	{
		x=v.x;
		y=v.y;
		z=v.z;
	}
	Vector3Type operator +(Vector3Type b) const
	{
		return Vector3Type(x+b.x,y+b.y,z+b.z);
	}
	Vector3Type operator +=(Vector3Type b)
	{
		return *this=*this+b;
	}
	Vector3Type operator -(Vector3Type b) const
	{
		return Vector3Type(x-b.x,y-b.y,z-b.z);
	}
	Vector3Type operator -=(Vector3Type b)
	{
		return *this=*this-b;
	}
	Vector3Type operator -() const
	{
		return Vector3Type(-x,-y,-z);
	}
	Vector3Type operator *(T b) const
	{
		return Vector3Type(x*b,y*b,z*b);
	}
	Vector3Type operator *=(T b)
	{
		return *this=*this*b;
	}
	Vector3Type operator /(T b) const
	{
		return Vector3Type(x/b,y/b,z/b);
	}
	Vector3Type operator /=(T b)
	{
		return *this=*this/b;
	}
	Vector3Type operator *(Vector3Type b) const
	{
		return Vector3Type(x*b.x,y*b.y,z*b.z);
	}
	Vector3Type operator *=(Vector3Type b)
	{
		return *this=*this*b;
	}
	Vector3Type operator /(Vector3Type b) const
	{
		return Vector3Type(x/b.x,y/b.y,z/b.z);
	}
	Vector3Type operator /=(Vector3Type b)
	{
		return *this=*this/b;
	}
	bool operator ==(Vector3Type b) const
	{
		return x==b.x && y==b.y && z==b.z;
	}
	bool operator !=(Vector3Type b) const
	{
		return !(*this==b);
	}
	bool within(Vector3Type min,Vector3Type max) const
	{
		return x>=min.x && x<=max.x && y>=min.y && y<=max.y && z>=min.z && z<=max.z;
	}
	template <class returnType=T>
	returnType d2() const
	{
		return x*x+y*y+z*z;
	}
	template <class returnType=T>
	returnType d() const
	{
		return sqrt(d2<returnType>());
	}
	Vector3Type n() const
	{
		T distance=d();
		if(distance==0) return Vector3Type(1,0,0);
		else return *this/distance;
	}
	Vector3Type<int> toPos3() const
	{
		return Vector3Type<int>(x,y,z);
	}
	Vector3Type<int> floor() const
	{
		return Vector3Type<int>(::floor(x),::floor(y),::floor(z));
	}
	Vector3Type<int> ceil() const
	{
		return Vector3Type<int>(::ceil(x),::ceil(y),::ceil(z));
	}
};

using Vector3=Vector3Type<double>;
using Vector3f=Vector3Type<float>;
using Pos3=Vector3Type<int>;

template <class T=float>
class VectorType
{
	public:
	typedef T value_type;
	vector<T> coordinates;
	VectorType(){}
	explicit VectorType(size_t size)
	{
		coordinates=vector<T>(size);
	}
	VectorType(size_t size,T initializationValue)
	{
		coordinates=vector<T>(size,initializationValue);
	}
	explicit VectorType(const vector<T>& _coordinates)
	{
		coordinates=_coordinates;
	}
	T& operator [](size_t index)
	{
		return coordinates[index];
	}
	T& at(size_t index)
	{
		return coordinates.at(index);
	}
	const T& operator [](size_t index) const
	{
		return coordinates[index];
	}
	const T& at(size_t index) const
	{
		return coordinates.at(index);
	}
	size_t size() const
	{
		return coordinates.size();
	}
	void operator +=(const VectorType& b)
	{
		size_t s=min(size(),b.size());
		for(size_t n=0;n<s;n++)
		{
			coordinates[n]+=b.coordinates[n];
		}
	}
	VectorType operator +(const VectorType& b) const
	{
		VectorType a=*this;
		a+=b;
		return a;
	}
	void operator -=(const VectorType& b)
	{
		size_t s=min(size(),b.size());
		for(size_t n=0;n<s;n++)
		{
			coordinates[n]-=b.coordinates[n];
		}
	}
	VectorType operator -(const VectorType& b) const
	{
		VectorType a=*this;
		a-=b;
		return a;
	}
	VectorType operator -() const
	{
		VectorType a=*this;
		for(size_t n=0;n<size();n++)
		{
			a.coordinates[n]=-a.coordinates[n];
		}
		return a;
	}
	void operator *=(T b)
	{
		for(size_t n=0;n<size();n++)
		{
			coordinates[n]*=b;
		}
	}
	VectorType operator *(T b) const
	{
		VectorType a=*this;
		a*=b;
		return a;
	}
	void operator /=(T b)
	{
		for(size_t n=0;n<size();n++)
		{
			coordinates[n]/=b;
		}
	}
	VectorType operator /(T b) const
	{
		VectorType a=*this;
		a/=b;
		return a;
	}
	void operator *=(const VectorType& b)
	{
		size_t s=min(size(),b.size());
		for(size_t n=0;n<s;n++)
		{
			coordinates[n]*=b.coordinates[n];
		}
	}
	VectorType operator *(const VectorType& b) const
	{
		VectorType a=*this;
		a*=b;
		return a;
	}
	void operator /=(const VectorType& b)
	{
		size_t s=min(size(),b.size());
		for(size_t n=0;n<s;n++)
		{
			coordinates[n]/=b.coordinates[n];
		}
	}
	VectorType operator /(const VectorType& b) const
	{
		VectorType a=*this;
		a/=b;
		return a;
	}
	bool operator ==(const VectorType& b) const
	{
		if(size()!=b.size()) return false;
		for(size_t n=0;n<size();n++)
		{
			if(coordinates[n]!=b.coordinates[n]) return false;
		}
		return true;
	}
	bool operator !=(const VectorType& b) const
	{
		return !(*this==b);
	}
	template <class returnType=T>
	returnType sum() const
	{
		returnType s=0;
		for(size_t n=0;n<size();n++)
		{
			s+=coordinates[n];
		}
		return s;
	}
	template <class returnType=T>
	returnType d2() const
	{
		VectorType p=*this;
		p*=*this;
		return p.sum<returnType>();
	}
	template <class returnType=T>
	returnType d() const
	{
		return sqrt(d2<returnType>());
	}
	VectorType n() const
	{
		T distance=d();
		if(distance==0)
		{
			VectorType v(size(),0);
			if(size()>0) v[0]=1;
			return v;
		}
		else return *this/distance;
	}
};

using Vector=VectorType<float>;

