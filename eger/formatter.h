#ifndef EGER_FORMATTER_H
#define EGER_FORMATTER_H

#include <sstream>
#include <iomanip>

namespace eger {

inline string human_readable_number(double d) {
    std::ostringstream os;
    bool minus = false;
    if(d < 0) { os << '-'; d *= -1; minus = true; }
    if(d < 0.0001) { os << "0.0001"; }
    else os << std::setprecision(4) << d;
    string r = os.str();
    return d < 0.1 ? (r.size() > (minus ? 7 : 6) ? r.substr(0, (minus ? 7 : 6)) : r) : r;
}

}

#endif
