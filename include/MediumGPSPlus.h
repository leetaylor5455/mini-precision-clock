#include <TinyGPSPlus.h>

class MediumGPSPlus: public TinyGPSPlus 
{
    private:
        int offset;

    public:
        MediumGPSPlus();
        void validate();
        void setOffset();
        void printInfo();
        int valid;
};