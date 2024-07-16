#pragma once


struct DataBlob
{
    virtual void resize() = 0;
    virtual size_t get_size() const = 0;
    virtual void* get_data_ptr() = 0;
    virtual const void* get_const_data_ptr() const = 0;
};