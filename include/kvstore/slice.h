#pragma once

#include<string>
#include<cstddef>
#include<cstring>

namespace kvstore{



class Slice
{
public:
    Slice() : data_(""), size_(0) {}
    
    Slice(const char* d,size_t n) : data_(d), size_(n){}

    Slice(const std :: string& s) : data_(s.data()), size_(s.size()){}

    Slice(const char* s) : data_(s), size_(strlen(s)){}

    const char* data() const {return data_;}
    size_t size() const {return size_;}

    bool empty() {return size_ == 0;}
    
    char operator[](size_t n){
        return data_[n];
    }

    void remove_prefix(size_t n){
        if (n<size_){
            data_ +=n;
            size_ -= n;
        }
        else{
            data_ = "";
            size_ = 0;
        }
    }

    std::string ToString() const {
        return std::string(data_,size_);
    }

    int compare(const Slice& b) const;

private:
    /* data */
    const char* data_;
    size_t size_;
};

inline bool operator==(const Slice& x,const Slice& y){
    return (x.size() == y.size()) && (memcmp(x.data(),y.data(),x.size())==0);
}

inline bool operator!=(const Slice& x,Slice& y){
    return !(x==y);
}

}