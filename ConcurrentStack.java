import java.util.concurrent.atomic.AtomicReference;

class Node<T> {
    T value;
    Node<T> next;

    Node(T value) {
        this.value = value;
    }   
}

public class ConcurrentStack<T> {
AtomicReference<Node<T>> top = new AtomicReference<>();

    public void push(T value) {
        Node<T> newNode = new Node<>(value);
        Node<T> oldTop;
        do {
            oldTop = top.get();
            newNode.next = oldTop;
        } while (!top.compareAndSet(oldTop, newNode));

    }

    public T pop() {
        Node<T> oldTop;
        Node<T> newTop;
        do {
            oldTop = top.get();
            if (oldTop == null) {
                return null; // Stack is empty
            }
            newTop = oldTop.next;
        } while (!top.compareAndSet(oldTop, newTop));
        return oldTop.value;    
    }

    public boolean isEmpty() {
        return top.get() == null; 
    }

    public T peek() {
        Node<T> currentTop = top.get(); 
        // Return the value of the top node without removing it
        return currentTop != null ? currentTop.value : null; 
    }

    public static void main(String[] args) {
        ConcurrentStack<Integer> stack = new ConcurrentStack<>();
        stack.push(1);
        stack.push(2);
        stack.push(3);

        System.out.println("Top element: " + stack.peek()); // Should print 3
        System.out.println("Popped element: " + stack.pop()); // Should print 3
        System.out.println("Is stack empty? " + stack.isEmpty()); // Should print false
        System.out.println("Popped element: " + stack.pop()); // Should print 2
        System.out.println("Popped element: " + stack.pop()); // Should print 1
        System.out.println("Is stack empty? " + stack.isEmpty()); // Should print true
    }   
}
