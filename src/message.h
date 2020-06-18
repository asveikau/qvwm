#ifndef MESSAGE_H_
#define MESSAGE_H_

#ifdef HAVE_VPRINTF
extern void QvwmError(const char* fmt, ...);
#else
#include <stdio.h>
#define QvwmError(fmt, args...) \
	printf("qvwm: "), printf(fmt, ## args), printf("\n")
#endif

#ifdef DEBUG
#include <stdio.h>
# define ASSERT(ex) {						\
  if(!(ex)) {							\
    fprintf(stderr,"Assertion failed: file \"%s\", line %d\n",	\
	    __FILE__, __LINE__);				\
    ::FinishQvwm();						\
    abort();							\
  }								\
}
#else
# define ASSERT(ex)
#endif

#endif // MESSAGE_H_
