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
#ifdef OMEGAGTE__BUILD__
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

#ifdef TARGET_VULKAN
#include <glm/glm.hpp>
#endif

#ifdef TARGET_METAL
#include <simd/simd.h>
#endif

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


    template<class Num_Ty,typename Angle_Ty = float>
    class  Vector2D_Base {
        Num_Ty i;
        Num_Ty j;
        public:
        Vector2D_Base(Num_Ty _i,Num_Ty _j):i(_i),j(_j){

        };
        static Vector2D_Base FromMagnitudeAndAngle(Num_Ty mag,Angle_Ty angle){
            Vector2D_Base v(cos(angle) * mag,sin(angle) * mag);
            return v;
        };
        Num_Ty & getI() const{
            return i;
        };
        Num_Ty & getJ() const{
            return j;
        };
        /// Get magnitude
        virtual Num_Ty mag(){
            return sqrt(pow(i,2) + pow(j,2));
        };
        /// Get angle relative to `i` !
        Angle_Ty angle(){
            return atan(j/i);
        };
        protected:
        void add_to_s(const Vector2D_Base<Num_Ty,Angle_Ty> & vector2d){
            i += vector2d.i;
            j += vector2d.j;
        };
        void subtract_to_s(const Vector2D_Base<Num_Ty,Angle_Ty> & vector2d){
            i -= vector2d.i;
            j -= vector2d.j;
        };

        Vector2D_Base<Num_Ty,Angle_Ty> add(const Vector2D_Base<Num_Ty,Angle_Ty> & vector2d){
            auto _i = i + vector2d.i;
            auto _j = j + vector2d.j;
            return Vector2D_Base(_i,_j);
        };

        Vector2D_Base<Num_Ty,Angle_Ty> subtract(const Vector2D_Base<Num_Ty,Angle_Ty> & vector2d){
            auto _i = i - vector2d.i;
            auto _j = j - vector2d.j;
            return Vector2D_Base(_i,_j);
        };
        public:
        virtual Vector2D_Base<Num_Ty,Angle_Ty> operator+(const Vector2D_Base<Num_Ty,Angle_Ty> & vec){
            return add(std::move(vec));
        };
        virtual Vector2D_Base<Num_Ty,Angle_Ty> operator-(const Vector2D_Base<Num_Ty,Angle_Ty> & vec){
            return subtract(std::move(vec));
        };
        void operator+=(const Vector2D_Base<Num_Ty,Angle_Ty> & vec){
            add_to_s(std::move(vec));
        };
        void operator-=(const Vector2D_Base<Num_Ty,Angle_Ty> & vec){
            subtract_to_s(std::move(vec));
        };
        Num_Ty dot(const Vector2D_Base<Num_Ty,Angle_Ty> & vec){
            return (i * vec.i) + (j * vec.j);
        };
    };
    
    typedef Vector2D_Base<float,float> FVector2D;
    typedef Vector2D_Base<int,float> IVector2D;

    template<class Num_Ty,typename Angle_Ty = float>
    class  Vector3D_Base : protected Vector2D_Base<Num_Ty,Angle_Ty> {
        typedef Vector2D_Base<Num_Ty,Angle_Ty> parent;
        Num_Ty k;
        public:
        Vector3D_Base(Num_Ty _i,Num_Ty _j,Num_Ty _k):parent(_i,_j),k(_k){};
        Vector3D_Base(const parent & vec):parent(vec),k(0){

        }
        static Vector3D_Base FromMagnitudeAndAngles(Num_Ty mag,Angle_Ty angle_v,Angle_Ty angle_h){
            Vector3D_Base v(parent::FromMagnitudeAndAngle(mag,angle_v));
            v.k = sin(angle_h) * mag;
            return v;
        };
        Num_Ty & getK() const{
            return k;
        };
        /// Get magnitude
        virtual Num_Ty mag() override{
            return std::sqrt((this->getI() * this->getI()) + (this->getJ() * this->getJ()) + (k * k));
        }
        /// Get the angle on the horizontal plane (Measured from `i`);
        Angle_Ty angle_h(){
            return parent::angle();
        }
        /// Get the angle on the vertical plane (Measured from `i + k`)
        Angle_Ty angle_v(){
            return atan(this->getJ()/sqrt(this->getI() * this->getI() + (k * k)));
        };
        protected:

        void add_to_s(const Vector3D_Base<Num_Ty,Angle_Ty> & vector3d){
            parent::add_to_s(vector3d);
            k += vector3d.k;
        };

        void subtract_to_s(const Vector3D_Base<Num_Ty,Angle_Ty> & vector3d){
            parent::subtract_to_s(vector3d);
            k -= vector3d.k;
        };

        void add_to_s_vec2(const Vector2D_Base<Num_Ty,Angle_Ty> & vector2d){
            parent::subtract_to_s(vector2d);
        };

        void sub_to_s_vec2(const Vector2D_Base<Num_Ty,Angle_Ty> & vector2d){
            parent::subtract_to_s(vector2d);
        };

        Vector3D_Base<Num_Ty,Angle_Ty> add(const Vector3D_Base<Num_Ty,Angle_Ty> & vector3d){
            Vector3D_Base v(this->getI(),this->getJ(),k);
            v.add_to_s(vector3d);
            return v;
        };

        Vector3D_Base<Num_Ty,Angle_Ty> subtract(const Vector3D_Base<Num_Ty,Angle_Ty> & vector3d){
            Vector3D_Base v(this->getI(),this->getJ(),k);
            v.subtract_to_s(vector3d);
            return v;
        };


        public:

        /// Operators
        /// @{
//        Vector3D_Base<Num_Ty,Angle_Ty> operator+(const Vector2D_Base<Num_Ty,Angle_Ty> & vec){
//            return add(std::move(vec));
//        };

        Vector3D_Base<Num_Ty,Angle_Ty> operator+(const Vector3D_Base<Num_Ty,Angle_Ty> & vec){
            return add(std::move(vec));
        };

//        Vector3D_Base<Num_Ty,Angle_Ty> operator-(const Vector2D_Base<Num_Ty,Angle_Ty> & vec){
//            return subtract(std::move(vec));
//        };

        Vector3D_Base<Num_Ty,Angle_Ty> operator-(const Vector3D_Base<Num_Ty,Angle_Ty> & vec){
            return subtract(std::move(vec));
        };


        void operator+=(const Vector3D_Base<Num_Ty,Angle_Ty> & vec){
            add_to_s(std::move(vec));
        };;

        void operator-=(const Vector3D_Base<Num_Ty,Angle_Ty> & vec){
            subtract_to_s(std::move(vec));
        };
        /// @}

        /// Vector Transformations
        /// @{
        Num_Ty dot(const Vector3D_Base<Num_Ty,Angle_Ty> & vec){
            return parent::dot(vec) + (k * vec.k);
        };

        Vector3D_Base<Num_Ty,Angle_Ty> cross(Vector3D_Base<Num_Ty,Angle_Ty> & vec){
            Num_Ty i_res = ((this->getI() * vec.k) - (k * vec.getI()));
            Num_Ty j_res = -((this->getI() * vec.k) - (k * vec.getI()));
            Num_Ty k_res = ((this->getI() * vec.getJ()) - (this->getJ() * vec.getI()));
            return Vector3D_Base(i_res,j_res,k_res);
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


    template<class Pt_Ty>
    class  GVectorPath_Base {
        public:
        struct Node {
            Pt_Ty *pt;
            Node *next = nullptr;
            Node(Pt_Ty *pt):pt(pt){};
            ~Node(){
                delete pt;
                if(next){
                    delete next;
                };
            };
        };

        struct  Segment {
            Pt_Ty ** pt_A;
            Pt_Ty ** pt_B;
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
        void _push_pt(const Pt_Ty & pt){
            Node ** pt_b = &first->next;
            unsigned idx = len;
            while(idx > 0){
                pt_b = &((*pt_b)->next);
                --idx;
            };
            *(pt_b) = new Node(new Pt_Ty(std::move(pt)));
            ++len;
        };
        public:
        void append(const Pt_Ty &pt){
            return _push_pt(pt);
        };
        void append(Pt_Ty &&pt){
            return _push_pt(pt);
        };
        std::string toStr(){
            std::ostringstream out_;
            out_ << "VectorPath Size:" << size() << std::endl;
            auto it = begin();
            
            while(it != end()){
                auto segment = *it;
                Pt_Ty *pt_A = *segment.pt_A;
                Pt_Ty *pt_B = *segment.pt_B;
                if(sizeof(Pt_Ty) == sizeof(GPoint2D)){
                    out_ << "Segment {" << "[x:" << pt_A->x << ",y:" << pt_A->y << "] [x:" << pt_B->x << ",y:" << pt_B->y << "] }"<< std::endl;
                }
                else if(sizeof(Pt_Ty) == sizeof(GPoint3D)){
//                        out_ << "Segment {" << "[x:" << pt_A->x << ",y:" << pt_A->y << ",z:" << pt_A->z << "] [x:" << pt_B->x << ",y:" << pt_B->y << ",z:" << pt_B->z << "] }"<< std::endl;
                };
                ++it;
            };
            return out_.str();
        };
        GVectorPath_Base(const Pt_Ty & start):first(new Node(new Pt_Ty(std::move(start)))),len(0),numPoints(1){};
        GVectorPath_Base(Pt_Ty &&start):first(new Node(new Pt_Ty(std::move(start)))),len(0),numPoints(1){};
        GVectorPath_Base(GVectorPath_Base<Pt_Ty> & other){
            Node *& pt_a = other.first;
            first = new Node(new Pt_Ty(*(pt_a->pt)));
            Node *&next = first->next;
            Node *& pt_b = other.first->next;
            next = new Node(new Pt_Ty(*(pt_b->pt)));
            unsigned idx = other.len-1;
            while(idx > 0){
                next = next->next;
                if(pt_b) {
                    pt_b = pt_b->next;
                    if(pt_b)
                        next = new Node(new Pt_Ty(*(pt_b->pt)));
                }
                --idx;
            };
            len = other.len;
            
        };
        GVectorPath_Base() = delete;
        GVectorPath_Base(const GVectorPath_Base<Pt_Ty> &) = delete;
        GVectorPath_Base(GVectorPath_Base<Pt_Ty> && other):first(other.first),len(other.len){};
        ~GVectorPath_Base(){
            delete first;
        };
    };
    
    typedef GVectorPath_Base<GPoint2D> GVectorPath2D;
    typedef GVectorPath_Base<GPoint3D> GVectorPath3D;



    template<class Ty,unsigned column,unsigned row>
   class Matrix {
   public:
       typedef Ty * row_pointer;
       typedef row_pointer * column_pointer;
       typedef unsigned size_type;

       class row_pointer_wrapper {
            row_pointer pt;
       public:
            row_pointer_wrapper(row_pointer _pt):pt(_pt){}

//            row_pointer_wrapper(const row_pointer_wrapper &) = delete;

            typedef row_pointer iterator;
            typedef Ty & reference;

            inline iterator begin(){
                return pt;
            }
            inline iterator end(){
                return pt + row;
            }

            inline Ty & at(size_type idx){
                assert(idx < row && "Cannot index row value at index");
                return pt[idx];
            }
            inline Ty & at(size_type idx) const{
                assert(idx < row && "Cannot index row value at index");
                return pt[idx];
            }
            inline Ty & operator[](size_type idx){
                return at(idx);
            }
            inline Ty & operator[](size_type idx) const{
                return at(idx);
            }
       };

       class row_pointer_wrapper_iterator {
           column_pointer pt;
       public:
           row_pointer_wrapper_iterator(column_pointer _pt):pt(_pt){

           }
           void operator +=(size_type n){
               pt += n;
           }
           void operator ++(){
               operator+=(1);
           }
           bool operator !=(const row_pointer_wrapper_iterator &other){
               return other.pt != pt;
           }
           row_pointer_wrapper operator *(){
               return {*pt};
           }

       };

       typedef row_pointer_wrapper_iterator iterator;
    private:
        column_pointer _data;

        inline void alloc_matrix_mem(column_pointer & dest){
            dest = new row_pointer [column];
            column_pointer _data_it = dest;
            unsigned _c = column;
            while(_c > 0){
                *_data_it = new Ty[row];
                ++_data_it;
                --_c;
            }
        }

        Matrix(){
            /// Alloc Matrix Mem
            alloc_matrix_mem(_data);

            /// Set all Matrix vals to 0
            column_pointer _data_it = _data;
            unsigned _c = column;
            while(_c > 0){
                row_pointer _row_it = *_data_it;
                unsigned _r = row;
                while(_r > 0){
                    *_row_it = 0;
                    --_r;
                }

                ++_data_it;
                --_c;
            }
        };

        template<unsigned _column,unsigned _row>
        static void copy_data_to(column_pointer from,column_pointer dest){
            column_pointer _data_it = dest;
            column_pointer _data_it_f = from;
            unsigned _c = _column;
            while(_c > 0){
                row_pointer _row_it = *_data_it;
                row_pointer _row_it_f = *_data_it_f;
                unsigned _r = _row;
                while(_r > 0){
                    *_row_it = *_row_it_f;
                    --_r;
                }

                ++_data_it;
                ++_data_it_f;
                --_c;
            }
        }
   public:
       inline iterator begin(){
            return {_data};
        }
        inline iterator end(){
            return {_data + column};
        }
       inline row_pointer_wrapper at(size_type idx){
           assert(idx < column && "Cannot index column pointer at index");
           return row_pointer_wrapper(_data[idx]);
       }
       inline row_pointer_wrapper at(size_type idx) const{
           assert(idx < column && "Cannot index column pointer at index");
           return row_pointer_wrapper{_data[idx]};
       }

       inline row_pointer_wrapper operator[](size_type idx){
            return at(idx);
       };

       inline row_pointer_wrapper operator[](size_type idx) const{
           return at(idx);
       };

       template<unsigned n_column,unsigned n_row>
       Matrix<Ty,n_column,n_row> resize(){
           Matrix<Ty,n_column,n_row> n;
           unsigned col_to_cpy,row_to_cpy;
           /// Downsizing Matrix Column Count
           if(n_column > column){
             col_to_cpy = column;
           }
           else {
               col_to_cpy = n_column;
           }
           /// Downsizing Matrix Row Count
           if(n_row > row){
               row_to_cpy = row;
           }
           else {
               row_to_cpy = n_row;
           }
           copy_data_to<col_to_cpy,row_to_cpy>(_data,n._data);
           return n;
       }

       template<unsigned o_column,unsigned o_row>
        Matrix(const Matrix<Ty,o_column,o_row> & other){
            alloc_matrix_mem(_data);
            unsigned col_to_cpy,row_to_cpy;
            /// Downsizing Matrix Column Count
            if(o_column > column){
                col_to_cpy = column;
            }
            else {
                col_to_cpy = o_column;
            }

            /// Downsizing Matrix Row Count
            if(o_row > row){
                row_to_cpy = row;
            }
            else {
                row_to_cpy = o_row;
            }
            copy_data_to(other._data,_data);
        };
       template<unsigned o_column,unsigned o_row>
        Matrix(Matrix<Ty,o_column,o_row> && other){
            alloc_matrix_mem(_data);
            unsigned col_to_cpy,row_to_cpy;
            /// Downsizing Matrix Column Count

            copy_data_to<column,row>(other._data,_data);
        };

        /// Construct a Matrix from a Vector2D
        Matrix(const Vector2D_Base<Ty> & vec){
            static_assert(row == 1 && column == 2 && "Cannot construct Matrix of size from Vector2D");
            alloc_matrix_mem(_data);

        }
        /// Construct a Matrix from a Vector3D
        Matrix(const Vector3D_Base<Ty> & vec){
            static_assert(row == 1 && column == 3 && "Cannot construct Matrix of size from Vector2D");
            alloc_matrix_mem(_data);

        }

       /** @brief Create an empty Matrix with the specified width and height.
           @param[in] h Height
           @param[in] w Width 
           @returns Matrix
       */
       static Matrix Create(){
           return Matrix();
       };
       static Matrix Identity(){
           static_assert(column == row && "Cannot construct an Identity Matrix");
           auto m = Create();

           for(unsigned _c = 0;_c < column;_c++){
                m[_c][_c] = 1;
           }
           return m;
       };
       ~Matrix(){
           unsigned _c = column;
           column_pointer _data_it = _data;
           while(_c > 0){
                delete [] (row_pointer)(*_data_it);
                --_c;
                ++_data_it;
           };
           delete [] _data;
       };
   };

    template<unsigned c,unsigned r>
    using IMatrix = Matrix<int,c,r>;

    template<unsigned c,unsigned r>
    using UMatrix = Matrix<unsigned int,c,r>;

    template<unsigned c,unsigned r>
    using FMatrix = Matrix<float,c,r>;

    template<unsigned c,unsigned r>
    using DMatrix = Matrix<double,c,r>;

    /// Single Row Matrices

    template<unsigned n>
    using IVec = IMatrix<n,1>;

    template<unsigned n>
    using UVec = UMatrix<n,1>;

    template<unsigned n>
    using FVec = FMatrix<n,1>;

    template<unsigned n>
    using DVec = DMatrix<n,1>;

    inline FVec<4> makeColor(float r,float g,float b,float a){
        auto m = FVec<4>::Create();
        m[0][0] = r;
        m[1][0] = g;
        m[2][0] = b;
        m[3][0] = a;
        return m;
    }

    /// Matrix Conversion Functions

    #define FLOAT_SIZE sizeof(float)

#ifdef TARGET_DIRECTX
#include <DirectXMath.h>



#elif defined(TARGET_METAL)

#define FLOAT2_SIZE sizeof(simd_float2)
#define FLOAT3_SIZE sizeof(simd_float3)
#define FLOAT4_SIZE sizeof(simd_float4)

#elif defined(TARGET_VULKAN)

    glm::vec2 float2(const FVec<2> & mat);
    glm::vec3 float3(const FVec<3> & mat);
    glm::vec4 float4(const FVec<4> & mat);
    
#endif


    template<class _Ty>
    using UniqueHandle = std::unique_ptr<_Ty>;

    template<class _Ty>
    using SharedHandle = std::shared_ptr<_Ty>;

_NAMESPACE_END_

#endif
