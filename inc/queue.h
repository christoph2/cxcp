
#ifndef CXCP_QUEUE_HPP
#define CXCP_QUEUE_HPP

template<typename T>
class Queue {
   public:

    explicit Queue(size_t size) {
        arr        = new T[size];
        m_capacity = size;
        m_front    = 0;
        m_rear     = -1;
        m_count    = 0;
    }

    Queue() = delete;

    ~Queue() {
        delete[] arr;
    }

    T pop() {
        if (isEmpty()) {
            exit(EXIT_FAILURE);
        }
        T x = arr[m_front];

        m_front = (m_front + 1) % m_capacity;
        m_count--;

        return x;
    }

    void push(const T &item) {
        if (isFull()) {
            exit(EXIT_FAILURE);
        }

        m_rear      = (m_rear + 1) % m_capacity;
        arr[m_rear] = item;
        m_count++;
    }
#if 0
    T peek() {
        if (isEmpty()) {
            exit(EXIT_FAILURE);
        }
        return arr[m_front];
    }
#endif
    size_t size() {
        return m_count;
    }

    bool isEmpty() {
        return (size() == 0);
    }

    bool isFull() {
        return (size() == m_capacity);
    }

   private:

    T     *arr;
    size_t m_capacity;
    int    m_front;
    int    m_rear;
    size_t m_count;
};

template<typename T>
class Observers {
   public:

    explicit Observers(size_t size) {
        arr        = new T[size];
        m_capacity = size;
        m_count    = 0;
    }

    Observers() = delete;

    ~Observers() {
        delete[] arr;
    }

    void append(const T &item) {
        if (isFull()) {
            exit(EXIT_FAILURE);
        }
        arr[m_count++] = item;
    }

    bool isFull() {
        return (size() == m_capacity);
    }

    size_t size() {
        return m_count;
    }

    template<typename V>
    void notify(const V &value) {
        for (size_t i = 0; i < m_capacity; ++i) {
            arr[i]->update(value);
        }
    }

   private:

    T     *arr;
    size_t m_capacity;
    size_t m_count;
};

#endif  // CXCP_QUEUE_HPP
