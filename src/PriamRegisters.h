#pragma once

namespace Priam {

template <int NUMREGS>
class RegisterValues
{
public:
    RegisterValues(const uint8_t vals[NUMREGS], bool valid) :isValid_(valid)
    {
        for (uint8_t i = 0; i < NUMREGS; i++)
            values_[i] = vals[i];
    };

    uint8_t GetRegisterValue(uint8_t i) const
    {
        if (validIndex(i))
            return values_[i]; 
        else
            return 0;
    }

    uint8_t NumRegisters() const
    {
        return NUMREGS; 
    }

    bool Valid() const
    {
        return isValid_;
    }
    
    private:
    bool validIndex(uint8_t index) const
    {
        return (index < NUMREGS);        
    }

    bool isValid_;
    uint8_t values_[NUMREGS];
};

};