/*
 * Pristine
 * printf: basic printf functionality
 * SPDX-License-Identifier MIT
 */

#include <printf.h>
#include <stdarg.h>
#include <common/string.h>
#include <stdint.h>

typedef enum {
    BASE_OCT = 8,
    BASE_DEC = 10,
    BASE_HEX = 16,
} PrintBase;

static int is_digit(char ch) {
    return (uint8_t)ch > 47 && (uint8_t)ch < 58;
}

static int is_length_modifier(char ch) {
    return 
        ch == 'h' ||
        ch == 'l' ||
        ch == 'L' ||
        ch == 'z' ||
        ch == 't' ||
        ch == 'j'  ;
}

static int is_format_specifier(char ch) {
    return 
        ch == 'd' ||
        ch == 'i' ||
        ch == 'u' ||
        ch == 'o' ||
        ch == 'x' ||
        ch == 'X' ||
        ch == 'f' ||
        ch == 'F' ||
        ch == 'e' ||
        ch == 'E' ||
        ch == 'g' ||
        ch == 'G' ||
        ch == 'c' ||
        ch == 's' ||
        ch == 'p' ||
        ch == 'n' ||
        ch == '%'  ;
}

static void vprintf_puts(const char* str) {
    while (*str) {
        putchar_(*str);
        str++;
    }
}

static void nprintf_puts(const char* str, size_t len) {
    while (len-- > 0) {
        putchar_(*str++);
    }
}

void printf_(const char *fmt, ...) {
    va_list list;
    va_start(list, fmt);
    vprintf_(fmt, list);
    va_end(list);
}

int64_t printf_resolve_signed(const char *lspec, uint8_t llen, va_list *list) {
    if (llen == 0) return va_arg(*list, int);
    if (llen == 1) {
        if (lspec[0] == 'h') return (short)va_arg(*list, int);
        if (lspec[0] == 'l') return va_arg(*list, long int);
        if (lspec[0] == 't') return va_arg(*list, ptrdiff_t);
        if (lspec[0] == 'z') return va_arg(*list, size_t);
        if (lspec[0] == 'j') return va_arg(*list, intmax_t);
    } else {
        if (lspec[0] == 'h' && lspec[1] == 'h') return (char)va_arg(*list, int);
        if (lspec[0] == 'l' && lspec[1] == 'l') return va_arg(*list, long long int);
    }
    return va_arg(*list, int);
}

uint64_t printf_resolve_unsigned(const char *lspec, uint8_t llen, va_list *list) {
    if (llen == 0) return va_arg(*list, unsigned int);
    if (llen == 1) {
        if (lspec[0] == 'h') return (unsigned short)va_arg(*list, int);
        if (lspec[0] == 'l') return va_arg(*list, long unsigned int);
        if (lspec[0] == 'z') return va_arg(*list, size_t);
        if (lspec[0] == 'j') return va_arg(*list, uintmax_t);
    } else {
        if (lspec[0] == 'h' && lspec[1] == 'h') return (unsigned char)va_arg(*list, int);
        if (lspec[0] == 'l' && lspec[1] == 'l') return va_arg(*list, long long unsigned int);
    }
    return va_arg(*list, unsigned int);
}

static char radix16[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};
static char radix10[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
static char radix8[] = { '0', '1', '2', '3', '4', '5', '6', '7' };

void vprintf_signed(int64_t num, uint8_t width, bool f_leftalign, bool f_space, bool f_sign, bool f_zeropad, bool f_alternate, bool uppercase, PrintBase base) {
    if (f_leftalign && f_zeropad) f_zeropad = false;
    if (f_space && f_zeropad) f_space = false;
    char pad = f_zeropad ? '0' : ' ';
    char buf[PRINTF_MAX_FORMAT_STACK];
    int buflen = 0;
    bool negative = false;

    if (num < 0) {
        negative = true; 
        num = -num;   
    }
    
    if (num == 0) {
            buf[0] = '0';
            buf[1] = '\0';
            buflen = 1;
    } else {
        if (base == BASE_DEC) {
            while (num > 0 && buflen < PRINTF_MAX_FORMAT_STACK - 1) {
                buf[buflen++] = radix10[(num % 10)];
                num /= 10;
            }
            buf[buflen] = '\0';
            strrev(buf);
        }
    }

    if (width < (buflen + negative)) width = buflen + negative;

    if (!f_leftalign) {
        if ((negative || f_sign) && f_zeropad) {
            putchar_(negative ? '-' : '+');
        }
        for (size_t i = 0; i < width - buflen - negative; i++) {
            putchar_(pad);
        }
        if ((negative || f_sign) && !f_zeropad) {
            putchar_(negative ? '-' : '+');
        }
        nprintf_puts(buf, buflen);
    } else {
        if (negative || f_sign) {
            putchar_(negative ? '-' : '+');
        }
        nprintf_puts(buf, buflen);
        for (size_t i = 0; i < width - buflen - negative; i++) {
            putchar_(pad);
        }
    }
}


void vprintf_unsigned(uint64_t num, uint8_t width, bool f_leftalign, bool f_space, bool f_sign, bool f_zeropad, bool f_alternate, bool uppercase, PrintBase base) {
    if (f_leftalign && f_zeropad) f_zeropad = false;
    if (f_space && f_zeropad) f_space = false;
    char pad = f_zeropad ? '0' : ' ';
    char buf[PRINTF_MAX_FORMAT_STACK];
    int buflen = 0;
    if (num == 0) {
            buf[0] = '0';
            buf[1] = '\0';
            buflen = 1;
    } else {
        if (base == BASE_DEC) {
            while (num > 0 && buflen < PRINTF_MAX_FORMAT_STACK - 1) {
                buf[buflen++] = radix10[(num % 10)];
                num /= 10;
            }
            buf[buflen] = '\0';
            strrev(buf);
        } else if (base == BASE_HEX) {
            while (num > 0 && buflen < PRINTF_MAX_FORMAT_STACK - 1) {
                buf[buflen++] = radix16[(num % 16) + uppercase * 16];
                num /= 16;
            }
            buf[buflen] = '\0';
            strrev(buf);
        } else {
            while (num > 0 && buflen < PRINTF_MAX_FORMAT_STACK - 1) {
                buf[buflen++] = radix8[(num % 8)];
                num /= 8;
            }
            buf[buflen] = '\0';
            strrev(buf);
        }
    }

    if (width < buflen) width = buflen;

    if (!f_leftalign) {
        for (size_t i = 0; i < width - buflen; i++) {
            putchar_(pad);
        }
        nprintf_puts(buf, buflen);
    } else {
        nprintf_puts(buf, buflen);
        for (size_t i = 0; i < width - buflen; i++) {
            putchar_(pad);
        }
    }
}

static void vprintf_ptr(void* ptr, bool f_zeropad) {
    uintptr_t num = (uintptr_t)ptr;
    char buf[PRINTF_MAX_FORMAT_STACK];
    int buflen = 0;
    if (num == 0) {
            buf[buflen++] = '0';
            buf[buflen++] = 'x';
            for (; buflen < 18 && buflen < PRINTF_MAX_FORMAT_STACK; buflen++) {
                buf[buflen] = '0';
            }
            buf[buflen] = '\0';
    } else {
        while (num > 0 && buflen < PRINTF_MAX_FORMAT_STACK - 1) {
            buf[buflen++] = radix16[(num % 16)];
            num /= 16;
        }
        for (; buflen < 16 && buflen < PRINTF_MAX_FORMAT_STACK; buflen++) {
            buf[buflen] = '0';
        }
        buf[buflen++] = 'x';
        buf[buflen++] = '0';
        buf[buflen] = '\0';
        strrev(buf);
    }
    nprintf_puts(buf, buflen);
}

void vprintf_str(const char *str, uint8_t width, bool f_leftalign) {
    size_t len = strlen(str);
    if (width < len) width = len;
    if (!f_leftalign) {
        for (size_t i = 0; i < width - len; i++) {
            putchar_(' ');
        }
        vprintf_puts(str);
    } else {
        vprintf_puts(str);
        for (size_t i = 0; i < width - len; i++) {
            putchar_(' ');
        }
    }
}

void vprintf_char(const char ch, uint8_t width, bool f_leftalign) {
    if (width < 1) width = 1;
    if (!f_leftalign) {
        for (size_t i = 0; i < width - 1; i++) {
            putchar_(' ');
        }
        putchar_(ch);
    } else {
        putchar_(ch);
        for (size_t i = 0; i < width - 1; i++) {
            putchar_(' ');
        }
    }
}

void vprintf_(const char *fmt, va_list list) {
    va_list args;
    va_copy(args, list);
    size_t len = strlen(fmt);
    for (size_t i = 0; i < len; i++) {
        const char ch = fmt[i];
        if (ch == '%') {
            i++; // skip initial '%'

            bool f_leftalign = false, f_sign = false, f_space = false, f_zeropad = false, f_alternate = false;
            for (size_t f = i; f < len; f++, i++) {
                if (fmt[f] == '+')
                    f_sign = true;
                else if (fmt[f] == '-')
                    f_leftalign = true;
                else if (fmt[f] == ' ')
                    f_space = true;
                else if (fmt[f] == '0')
                    f_zeropad = true;
                else if (fmt[f] == '#')
                    f_alternate = true;
                else
                    break;
            }

            uint8_t widx = 0;
            char wspec[PRINTF_MAX_WIDTH_MODIFIER] = {0};
            for (size_t w = i; w < len && widx < PRINTF_MAX_WIDTH_MODIFIER - 1; w++, i++, widx++) {
                if (is_digit(fmt[w]))
                    wspec[widx] = fmt[w];
                else
                    break;
            }
            
            uint8_t width = 0;
            if (widx != 0) {
                wspec[widx] = '\0';
                size_t wlen = strlen(wspec);
                for (size_t w = 0; w < wlen; w++) {
                    uint8_t mul = 1;
                    for (size_t m = w; m < wlen - 1; m++, mul *= 10);
                    width += (wspec[w] - 48) * mul;
                }
            }

            // Lets skip precision for now

            uint8_t lidx = 0;
            char lspec[PRINTF_MAX_LENGTH_MODIFIER] = {0};
            for (size_t l = i; l < len && lidx < PRINTF_MAX_LENGTH_MODIFIER; l++, i++, lidx++) {
                if (is_length_modifier(fmt[l]))
                    lspec[lidx] = fmt[l];
                else
                    break;
            }

            char specifier = '\0';
            if (is_format_specifier(fmt[i])) {
                specifier = fmt[i];
            }

            if (specifier == '%')
                putchar_('%');
            else {
                if (specifier == 's') {
                    const char* str = va_arg(args, const char*);
                    vprintf_str(str, width, f_leftalign);
                } else if (specifier == 'c') {
                    char ch = (char)va_arg(args, int);
                    vprintf_char(ch, width, f_leftalign);
                } else if (specifier == 'p') {
                    void* addr = va_arg(args, void*);
                    vprintf_ptr(addr, f_zeropad);
                } else if (specifier == 'd' || specifier == 'i') {
                    int64_t num = printf_resolve_signed(lspec, lidx, &args);
                    vprintf_signed(num, width, f_leftalign, f_space, f_sign, f_zeropad, f_alternate, false, BASE_DEC);
                } else if (specifier == 'u') {
                    int64_t unum = printf_resolve_unsigned(lspec, lidx, &args);
                    vprintf_unsigned(unum, width, f_leftalign, f_space, f_sign, f_zeropad, f_alternate, false, BASE_DEC);
                } else if (specifier == 'x' || specifier == 'X') {
                    int64_t unum = printf_resolve_unsigned(lspec, lidx, &args);
                    vprintf_unsigned(unum, width, f_leftalign, f_space, f_sign, f_zeropad, f_alternate, specifier == 'X', BASE_HEX);
                } else if (specifier == 'o') {
                    int64_t unum = printf_resolve_unsigned(lspec, lidx, &args);
                    vprintf_unsigned(unum, width, f_leftalign, f_space, f_sign, f_zeropad, f_alternate, false, BASE_OCT);
                }
            }
            continue;
        }
        putchar_(ch);
    }
}