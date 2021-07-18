#include <memory>
#include <vector>
#include <cmath>
#include <cassert>
#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <new>

#include <omega-common/common.h>


#ifndef OMEGAGTE_GTEBASE_H
#define OMEGAGTE_GTEBASE_H

#ifdef TARGET_DIRECTX
#include <Windows.h>
#ifdef __BUILD__
#define OMEGAGTE_EXPORT __declspec( dllexport ) 
#else 
#define OMEGAGTE_EXPORT __declspec( dllimport ) 
#endif 
#else 
#define OMEGAGTE_EXPORT
#endif

#define IN_INIT_SCOPE friend GTE Init();

#define _NAMESPACE_BEGIN_ namespace OmegaGTE {
#define _NAMESPACE_END_ }

_NAMESPACE_BEGIN_
    using namespace OmegaCommon;
    typedef enum : int {
        CodeOk,
        CodeFailed
    } StatusCode;

    /// A vector that preallocates a certain amount of memory but can be resized at any time.
    template<class T>
    class VectorHeap {
        T * _data;
    public:
        using size_type = unsigned;
        using iterator = T *;
        using const_iterator = const T*;
        using reference = T &;
        using const_reference = const T &;
    private:
        size_type maxSize;
        size_type len;
        void _push_el(const T & el){
            assert(len < maxSize && "Maximum size of VectorHeap has been hit, please resize.");
            auto dest = _data + len;
            memmove(_data + len,&el,sizeof(T));
            ++len;
        };
    public:
        iterator begin(){ return iterator(_data);};
        iterator end(){ return iterator(_data + len);};
        const_iterator cbegin() const{ return const_iterator(_data);};
        const_iterator cend() const{ return const_iterator(_data + len);};

        reference first(){ return begin()[0];};
        reference last(){ return end()[-1];};

        bool empty(){return len == 0;};

        bool full(){return len == maxSize;}

        void resize(unsigned newSize){
            auto temp = ::new T[newSize];
            std::move(begin(),end(),temp);
            delete [] _data;
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

        explicit VectorHeap(unsigned maxSize):maxSize(maxSize),len(0),_data(::new T[maxSize]){
            
        };
        VectorHeap(const VectorHeap & other){
            maxSize = other.maxSize;
            len = other.len;
            // _data = ::new;
            memcpy(_data,other.cbegin(),other.len * sizeof(T));
        };

        VectorHeap(VectorHeap & other){
            maxSize = other.maxSize;
            len = other.len;
            _data = ::new T[maxSize];
            memcpy(_data,other.begin(),other.len * sizeof(T));
        };
        // VectorHeap operator=(VectorHeap &other){
        //     maxSize = other.maxSize;
        //     len = other.len;
        //     _data = alloc.allocate(maxSize);
        //     std::copy(other.begin(),other.end(),begin());
        // };
        VectorHeap(VectorHeap && other){
            maxSize = other.maxSize;
            len = other.len;
            _data = ::new T[maxSize];
            memcpy(_data,other._data,other.len * sizeof(T));
        };
        ~VectorHeap(){
            if(_data != nullptr){
                for(auto & obj : *this){
                    obj.~T();
                };
                delete [] _data;
            }
        };
    };

    OMEGAGTE_EXPORT extern const long double PI;


    template<class _Num_Ty,typename _Angle_Ty>
    class  Vector2D_Base {
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
    class  Vector3D_Base {
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
        float x,y,z = 0;
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
    class  GVectorPath_Base {
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

        struct  Segment {
            _Pt_Ty ** pt_A;
            _Pt_Ty ** pt_B;
        };

        class  Path_Iterator {
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
                return {&(pt_A->pt),&(pt_B->pt)};
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
       std::vector<std::vector<_Ty> *> rows;
    private:
        Matrix(unsigned h,unsigned w){
            /// Initialize Matrix with zeros.
            // MessageBoxA(GetForegroundWindow(),"Creating Matrix",NULL,MB_OK);
            while(h > 0){
                auto n_w = w;
                auto vec = new std::vector<_Ty>();
                //  MessageBoxA(GetForegroundWindow(),"Creating Matrix -- New Vector",NULL,MB_OK);
                while(n_w > 0) {
                    vec->push_back(0.f);
                    --n_w;
                }
                rows.push_back(vec);
                --h;
            }
            //  MessageBoxA(GetForegroundWindow(),"Finishing Creating Matrix",NULL,MB_OK);
        };
   public:
        Matrix(const Matrix & other){
            for(auto & col : other.rows){
                auto vec = new std::vector<_Ty>();
                for(auto & val : *col){
                    vec->push_back(val);
                };
                rows.push_back(vec);
            };
        };
        Matrix(Matrix && other){
            for(auto & col : other.rows){
                auto vec = new std::vector<_Ty>();
                for(auto & val : *col){
                    vec->push_back(val);
                };
                rows.push_back(vec);
            };
        };
        void setValueAt(unsigned row,unsigned column,_Ty val){
            auto row_it = rows.begin() + (row-1);
            auto column_it = ((*row_it)->begin()) + (column-1);
            *column_it = val;
        };
        _Ty & valueAt(unsigned row,unsigned column){
            auto row_it = rows.begin() + (row-1);
            auto column_it = ((*row_it)->begin()) + (column-1);
            return *column_it;
        };
       /** @brief Create an empty Matrix with the specified width and height.
           @param[in] h Height
           @param[in] w Width 
           @returns Matrix
       */
       static Matrix Create(unsigned h,unsigned w){
           return {h,w};
       };
       static Matrix Identity(unsigned h,unsigned w){
           auto m = Create(h,w);
           return {h,w};
       };
       static Matrix FromVector2D(Vector2D_Base<_Ty,float> vector){
           
       };
       static Matrix Color(float r,float g,float b,float a){
            // MessageBoxA(GetForegroundWindow(),"Create Matrix",NULL,MB_OK);
           auto m = Create(1,4);
        //    MessageBoxA(GetForegroundWindow(),"Created Matrix",NULL,MB_OK);
           std::cout << "Created Matrix" << std::endl;
           m.setValueAt(1,1,r);
           m.setValueAt(1,2,g);
           m.setValueAt(1,3,b);
           m.setValueAt(1,4,a);
           std::cout << "Return Matrix" << std::endl;
           return std::move(m);
       };
       ~Matrix(){
           for(auto & r : rows){
               delete r;
           };
       };
   };

   typedef Matrix<float> FMatrix;


    template<class _Ty>
    using UniqueHandle = std::unique_ptr<_Ty>;

    template<class _Ty>
    using SharedHandle = std::shared_ptr<_Ty>;

_NAMESPACE_END_

#endif
