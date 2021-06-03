#include <memory>
#include <vector>
#include <cmath>
#include <string>
#include <sstream>

#ifndef OMEGAGTE_GTEBASE_H
#define OMEGAGTE_GTEBASE_H

#ifdef TARGET_DIRECTX
#define OMEGAGTE_EXPORT __declspec( dllexport ) 
#else 
#define OMEGAGTE_EXPORT
#endif

#define IN_INIT_SCOPE friend GTE Init();

#define _NAMESPACE_BEGIN_ namespace OmegaGTE {
#define _NAMESPACE_END_ }

_NAMESPACE_BEGIN_

    typedef enum : int {
        CodeOk,
        CodeFailed
    } StatusCode;

    /// A vector that preallocates a certain amount of memory but can be resized at any time.
    template<class T>
    class OMEGAGTE_EXPORT VectorHeap {
        std::allocator<T> alloc;
        T * _data;
    public:
        using size_type = unsigned;
        using iterator = T *;
        using reference = T &;
    private:
        size_type maxSize;
        size_type len;
        void _push_el(const T & el){
            assert(len < maxSize && "Maxium size of VectorHeap has been hit, please resize.");
            auto dest = _data + len;
            memmove(_data + len,&el,sizeof(T));
            ++len;
        };
    public:
        iterator begin(){ return iterator(_data);};
        iterator end(){ return iterator(_data + len);};

        reference first(){ return begin()[0];};
        reference last(){ return end()[-1];};

        bool empty(){return len == 0;};

        bool full(){return len == maxSize;}

        void resize(unsigned newSize){
            auto temp = alloc.allocate(newSize);
            std::move(begin(),end(),temp);
            alloc.deallocate(_data,maxSize);
            maxSize = newSize;
            _data = temp;
        };

        void push(const T & el){ _push_el(el);};
        void push(T && el){ _push_el(el);};
        void pop(){
            assert(len > 0 && "Cannot call pop() when VectorHeap is empty.");
            auto _end = last();
            _end.~T();
            --len;
        };
        size_type capacity(){
            return maxSize;
        };
        size_type length(){ return len; };

        VectorHeap(unsigned maxSize){
           _data = alloc.allocate(maxSize);
        };
        ~VectorHeap(){
            for(auto & obj : *this){
                obj.~T();
            };
            alloc.deallocate(_data,maxSize);
        };
    };

    extern const long double PI;


    template<class _Num_Ty,typename _Angle_Ty>
    class  OMEGAGTE_EXPORT Vector2D_Base {
        _Num_Ty i;
        _Num_Ty j;
        public:
        Vector2D_Base(_Num_Ty _i,_Num_Ty _j):i(_i),j(_j){};
        Vector2D_Base(_Num_Ty mag,_Angle_Ty angle,bool useComp){
            i = cos(angle) * mag;
            j = sin(angle) * mag;
        };
        _Num_Ty & getI(){
            return i;
        };
        _Num_Ty & getJ(){
            return j;
        };
        /// Get magnitude
        _Num_Ty mag(){
            return sqrt(pow(i,2) + pow(j,2));
        };
        /// Get angle relative to `i` !
        _Angle_Ty angle(){
            return atan(j/i);
        };
        private:
        void add_to_s(const Vector2D_Base<_Num_Ty,_Angle_Ty> & vector2d){
            i += vector2d.i;
            j += vector2d.j;
        };
        void subtract_to_s(const Vector2D_Base<_Num_Ty,_Angle_Ty> & vector2d){
            i -= vector2d.i;
            j -= vector2d.j;
        };
        Vector2D_Base<_Num_Ty,_Angle_Ty> add(const Vector2D_Base<_Num_Ty,_Angle_Ty> & vector2d){
            auto _i = i + vector2d.i;
            auto _j = j + vector2d.j;
            return Vector2DBase(_i,_j);
        };
        Vector2D_Base<_Num_Ty,_Angle_Ty> subtract(const Vector2D_Base<_Num_Ty,_Angle_Ty> & vector2d){
            auto _i = i - vector2d.i;
            auto _j = j - vector2d.j;
            return Vector2DBase(_i,_j);
        };
        public:
        Vector2D_Base<_Num_Ty,_Angle_Ty> operator+(const Vector2D_Base<_Num_Ty,_Angle_Ty> & vec){
            return add(std::move(vec));
        };
        Vector2D_Base<_Num_Ty,_Angle_Ty> operator-(const Vector2D_Base<_Num_Ty,_Angle_Ty> & vec){
            return subtract(std::move(vec));
        };
        void operator+=(const Vector2D_Base<_Num_Ty,_Angle_Ty> & vec){
            add_to_s(std::move(vec));
        };
        void operator-=(const Vector2D_Base<_Num_Ty,_Angle_Ty> & vec){
            subtract_to_s(std::move(vec));
        };
        _Num_Ty dot(const Vector2D_Base<_Num_Ty,_Angle_Ty> & vec){
            return (i * vec.i) + (j * vec.j);
        };
    };
    
    typedef Vector2D_Base<float,float> FVector2D;
    typedef Vector2D_Base<int,float> IVector2D;

    template<class _Num_Ty,typename _Angle_Ty>
    class  OMEGAGTE_EXPORT Vector3D_Base {
        _Num_Ty i,j,k;
        public:
        Vector3D_Base(_Num_Ty _i,_Num_Ty _j,_Num_Ty _k):i(_i),j(_j),k(_k){};
        Vector3D_Base(_Num_Ty mag,_Angle_Ty angle_v,_Angle_Ty angle_h,bool noComp){
            i = cos(angle_h) * mag;
            j = sin(angle_v) * mag;
            k = sin(angle_h) * mag;
        };
        _Num_Ty & getI(){
            return i;
        };
        _Num_Ty & getJ(){
            return j;
        };
        _Num_Ty & getK(){
            return k;
        };
        /// Get magnitude
        _Num_Ty mag(){
            return sqrt(pow(i,2) + pow(j,2) + pow(k,2));
        }
        /// Get the angle on the horizontal plane (Measured from `i`)
        _Angle_Ty angle_h(){
            return atan(k/i);
        };
        /// Get the angle on the verical plane (Measured from `i + k`)
        _Angle_Ty angle_v(){
            return atan(j,sqrt(pow(i,2) + pow(k,2)));
        };
        private:
        void add_to_s(const Vector2D_Base<_Num_Ty,_Angle_Ty> & vector2d){
            i += vector2d.i;
            j += vector2d.j;
        };

        void add_to_s(const Vector3D_Base<_Num_Ty,_Angle_Ty> & vector3d){
            i += vector3d.i;
            j += vector3d.j;
            k += vector3d.k;
        };

        void subtract_to_s(const Vector2D_Base<_Num_Ty,_Angle_Ty> & vector2d){
            i -= vector2d.i;
            j -= vector2d.j;
        };

        void subtract_to_s(const Vector3D_Base<_Num_Ty,_Angle_Ty> & vector3d){
            i -= vector3d.i;
            j -= vector3d.j;
            k -= vector3d.k;
        };


        Vector3D_Base<_Num_Ty,_Angle_Ty> add(const Vector2D_Base<_Num_Ty,_Angle_Ty> & vector2d){
            auto _i = i + vector2d.i;
            auto _j = j + vector2d.j;
            return Vector3DBase(_i,_j,k);
        };

        Vector3D_Base<_Num_Ty,_Angle_Ty> add(const Vector3D_Base<_Num_Ty,_Angle_Ty> & vector3d){
            auto _i = i + vector3d.i;
            auto _j = j + vector3d.j;
            auto _k = k + vector3d.k;
            return Vector3DBase(_i,_j,_k);
        };

        Vector3D_Base<_Num_Ty,_Angle_Ty> subtract(const Vector2D_Base<_Num_Ty,_Angle_Ty> & vector2d){
            auto _i = i - vector2d.i;
            auto _j = j - vector2d.j;
            return Vector3DBase(_i,_j,k);
        };

        Vector3D_Base<_Num_Ty,_Angle_Ty> subtract(const Vector3D_Base<_Num_Ty,_Angle_Ty> & vector3d){
            auto _i = i - vector3d.i;
            auto _j = j - vector3d.j;
            auto _k = k - vector3d.k;
            return Vector3DBase(_i,_j,_k);
        };


        public:

        /// Operators
        /// @{
        Vector3D_Base<_Num_Ty,_Angle_Ty> operator+(const Vector2D_Base<_Num_Ty,_Angle_Ty> & vec){
            return add(std::move(vec));
        };

        Vector3D_Base<_Num_Ty,_Angle_Ty> operator+(const Vector3D_Base<_Num_Ty,_Angle_Ty> & vec){
            return add(std::move(vec));
        };

        Vector3D_Base<_Num_Ty,_Angle_Ty> operator-(const Vector2D_Base<_Num_Ty,_Angle_Ty> & vec){
            return subtract(std::move(vec));
        };

        Vector3D_Base<_Num_Ty,_Angle_Ty> operator-(const Vector3D_Base<_Num_Ty,_Angle_Ty> & vec){
            return subtract(std::move(vec));
        };

        

        void operator+=(const Vector2D_Base<_Num_Ty,_Angle_Ty> & vec){
            add_to_s(std::move(vec));
        };

        void operator+=(const Vector3D_Base<_Num_Ty,_Angle_Ty> & vec){
            add_to_s(std::move(vec));
        };

        void operator-=(const Vector2D_Base<_Num_Ty,_Angle_Ty> & vec){
            subtract_to_s(std::move(vec));
        };

        void operator-=(const Vector3D_Base<_Num_Ty,_Angle_Ty> & vec){
            subtract_to_s(std::move(vec));
        };
        /// @}

        /// Vector Transformations
        /// @{
        _Num_Ty dot(const Vector2D_Base<_Num_Ty,_Angle_Ty> & vec){
            return (i * vec.i) + (j * vec.j) + (k * 0);
        };
        _Num_Ty dot(const Vector3D_Base<_Num_Ty,_Angle_Ty> & vec){
            return (i * vec.i) + (j * vec.j) + (k * vec.k);
        };

        Vector3D_Base<_Num_Ty,_Angle_Ty> cross(const Vector3D_Base<_Num_Ty,_Angle_Ty> & vec){
            _Num_Ty i_res = ((j * vec.k) - (k * vec.j));
            _Num_Ty j_res = -((i * vec.k) - (k * vec.i));
            _Num_Ty k_res = ((i * vec.j) - (j * vec.i));
            return Vector3DBase(i_res,j_res,k_res);
        };
        /// @}
    };

    typedef Vector3D_Base<int,float>  IVector3D;
    typedef Vector3D_Base<float,float> FVector3D;



    struct  OMEGAGTE_EXPORT GPoint2D {
        float x,y;
    };

    struct  OMEGAGTE_EXPORT GArc {
        GPoint2D center;
        float radians;
        unsigned radius_x;
        unsigned radius_y;
    };
    struct  OMEGAGTE_EXPORT GPoint3D {
        float x,y,z;
    };

    struct  OMEGAGTE_EXPORT GRect {
        GPoint2D pos;
        float w,h;
    };

    struct  OMEGAGTE_EXPORT GRoundedRect {
        GPoint2D pos;
        float w,h,rad_x,rad_y;
    };

    struct  OMEGAGTE_EXPORT GRectangularPrism {
        GPoint3D pos;
        float w,h,d;
    };

    struct  OMEGAGTE_EXPORT GCylinder {
        GPoint3D pos;
        float r,h;
    };

    struct  OMEGAGTE_EXPORT GPyramid {
        float x,y,z,w,d,h;
    };

    struct  OMEGAGTE_EXPORT GCone {
        float x,y,z,r,h;
    };

    struct  OMEGAGTE_EXPORT GEllipsoid {
        float x,y,z,rad_x,rad_y,rad_z;
    };


    template<class _Pt_Ty>
    class  OMEGAGTE_EXPORT GVectorPath_Base {
        public:
        struct Node {
            _Pt_Ty *pt;
            Node *next = nullptr;
            Node(_Pt_Ty *pt):pt(pt){};
            ~Node(){
                delete pt;
                if(next){
                    delete next;
                };
            };
        };

        struct  OMEGAGTE_EXPORT Segment {
            _Pt_Ty ** pt_A;
            _Pt_Ty ** pt_B;
        };

        class  OMEGAGTE_EXPORT Path_Iterator {
            Node *pt_A;
            Node *pt_B;
            unsigned pos;
            public:
            Path_Iterator(Node *_data):pt_A(_data),pt_B(pt_A->next),pos(0){
                
            };
            Path_Iterator operator++(){
                pt_A = pt_A->next;
                pt_B = pt_B->next;
                ++pos;
                return *this;
            };
            bool operator==(const Path_Iterator & r){
//                    std::cout << "Self Pos:" << pos << ", Other Pos:" << r.pos << std::endl;
                return pos == r.pos;
            };
            bool operator!=(const Path_Iterator & r){
                return !operator==(r);
            };
            Path_Iterator operator+(unsigned num){
                while(num > 0){
                    operator++();
                    --num;
                };
                return *this;
            };
            Segment operator*(){
                Segment seg;
                seg.pt_A = &pt_A;
                seg.pt_b = &pt_B;
                return seg;
            };

        };
        using size_ty = unsigned;
        private:
        Node *first;
        float numPoints = 0;
        size_ty len = 0;
        public:
        using iterator = Path_Iterator;
        iterator begin(){
            return iterator(first);
        };
        iterator end(){
            return iterator(first) + (len);
        };
        const size_ty & size(){ return len;};
        private:
        void _push_pt(const _Pt_Ty & pt){
            Node ** pt_b = &first->next;
            unsigned idx = len;
            while(idx > 0){
                pt_b = &((*pt_b)->next);
                --idx;
            };
            *(pt_b) = new Node(new _Pt_Ty(std::move(pt)));
            ++len;
        };
        public:
        void append(const _Pt_Ty &pt){
            return _push_pt(pt);
        };
        void append(_Pt_Ty &&pt){
            return _push_pt(pt);
        };
        std::string toStr(){
            std::ostringstream out_;
            out_ << "VectorPath Size:" << size() << std::endl;
            auto it = begin();
            
            while(it != end()){
                auto segment = *it;
                _Pt_Ty *pt_A = *segment.pt_A;
                _Pt_Ty *pt_B = *segment.pt_B;
                if(sizeof(_Pt_Ty) == sizeof(GPoint2D)){
                    out_ << "Segment {" << "[x:" << pt_A->x << ",y:" << pt_A->y << "] [x:" << pt_B->x << ",y:" << pt_B->y << "] }"<< std::endl;
                }
                else if(sizeof(_Pt_Ty) == sizeof(GPoint3D)){
//                        out_ << "Segment {" << "[x:" << pt_A->x << ",y:" << pt_A->y << ",z:" << pt_A->z << "] [x:" << pt_B->x << ",y:" << pt_B->y << ",z:" << pt_B->z << "] }"<< std::endl;
                };
                ++it;
            };
            return out_.str();
        };
        GVectorPath_Base(const _Pt_Ty & start):first(new Node(new _Pt_Ty(std::move(start)))),len(0),numPoints(1){};
        GVectorPath_Base(_Pt_Ty &&start):first(new Node(new _Pt_Ty(std::move(start)))),len(0),numPoints(1){};
        GVectorPath_Base(GVectorPath_Base<_Pt_Ty> & other){
            Node *& pt_a = other.first;
            first = new Node(new _Pt_Ty(*(pt_a->pt)));
            Node *&next = first->next;
            Node *& pt_b = other.first->next;
            next = new Node(new _Pt_Ty(*(pt_b->pt)));
            unsigned idx = other.len-1;
            while(idx > 0){
                next = next->next;
                if(pt_b) {
                    pt_b = pt_b->next;
                    if(pt_b)
                        next = new Node(new _Pt_Ty(*(pt_b->pt)));
                }
                --idx;
            };
            len = other.len;
            
        };
        GVectorPath_Base() = delete;
        GVectorPath_Base(const GVectorPath_Base<_Pt_Ty> &) = delete;
        GVectorPath_Base(GVectorPath_Base<_Pt_Ty> && other):first(other.first),len(other.len){};
        ~GVectorPath_Base(){
            delete first;
        };
    };
    
    typedef GVectorPath_Base<GPoint2D> GVectorPath2D;
    typedef GVectorPath_Base<GPoint3D> GVectorPath3D;



   template<class _Ty>
   class Matrix {
       VectorHeap<VectorHeap<_Ty>> * rows;
    private:
        Matrix(unsigned h,unsigned w){
            rows = new VectorHeap<VectorHeap<_Ty>>(w);
            /// Intialize Matrix with zeros.
            while(w > 0){
                auto n_h = h;
                rows->push({h});
                while(n_h > 0) {
                    rows->last().push(0.f);
                    --n_h;
                }
                --w;
            }
        };
   public:
        _Ty & valueAt(unsigned row,unsigned column){
            auto row_it = rows->begin() + (row-1);
            auto column_it = (row_it->begin()) + (column-1);
            return *column_it;
        };
       /** @brief Create an empty Matrix with the specified width and height.
           @param[in] h Height
           @param[in] w Width 
           @returns Matrix
       */
       static Matrix Create(unsigned h,unsigned w){
           return Matrix(h,w);
       };
       static Matrix Identity(unsigned h,unsigned w){
           auto m = Create(h,w);
           
       };
       static Matrix FromVector2D(Vector2D_Base<_Ty,float> vector){
           
       };
       ~Matrix(){
           delete rows;
       };
//        static Matrix FromVector3D();
//        static Matrix Create(std::initializer_list<std::initializer_list<_Ty>> data);
   };

   typedef Matrix<float> FMatrix;


    template<class _Ty>
    using UniqueHandle = std::unique_ptr<_Ty>;

    template<class _Ty>
    using SharedHandle = std::shared_ptr<_Ty>;

_NAMESPACE_END_

#endif
