class Block {
    int start;
    int size;
    boolean free;
    String id;
    int requested;
    Block prev;
    Block next;

    Block(int start, int size) {
        this.start = start;
        this.size = size;
        this.free = true;
        this.id = "";
        this.requested = 0;
        this.prev = null;
        this.next = null;
    }
}
