#include "runqueue.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>

int main(void) {
    TCB dummies[16];
    TCB *tp[16];
    for (int i = 0; i < 16; i++) {
        tp[i] = &dummies[i];
    }

    /* rq_init(0) -> -EINVAL */
    assert(rq_init(0) == -EINVAL);

    /* basic FIFO, empty/full edge cases */
    assert(rq_init(4) == 0);
    assert(rq_dequeue() == NULL);
    assert(rq_enqueue(NULL) == -EINVAL);
    assert(rq_enqueue(tp[0]) == 0);
    assert(rq_enqueue(tp[1]) == 0);
    assert(rq_enqueue(tp[2]) == 0);
    assert(rq_size() == 3);
    assert(rq_enqueue(tp[3]) == 0);
    assert(rq_size() == 4);
    assert(rq_enqueue(tp[4]) == -EAGAIN);
    assert(rq_dequeue() == tp[0]);
    assert(rq_dequeue() == tp[1]);
    assert(rq_dequeue() == tp[2]);
    assert(rq_dequeue() == tp[3]);
    assert(rq_dequeue() == NULL);
    assert(rq_size() == 0);

    /* rq_destroy then rq_init works */
    rq_destroy();
    assert(rq_init(2) == 0);
    assert(rq_enqueue(tp[0]) == 0);
    rq_destroy();

    /* 10,000 enqueue/dequeue cycles across the wrap boundary keep order */
    assert(rq_init(8) == 0);
    for (int i = 0; i < 10000; i++) {
        int a = i % 16;
        int b = (i + 1) % 16;
        assert(rq_enqueue(tp[a]) == 0);
        assert(rq_enqueue(tp[b]) == 0);
        assert(rq_dequeue() == tp[a]);
        assert(rq_dequeue() == tp[b]);
    }
    assert(rq_size() == 0);
    rq_destroy();

    printf("PASS test_runqueue\n");
    return 0;
}
