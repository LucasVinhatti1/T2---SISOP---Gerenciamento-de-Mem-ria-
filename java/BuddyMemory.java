public class BuddyMemory {

    private Block head;
    private int memoryTotal;

    public void init(int totalSize) {
        memoryTotal = totalSize;
        head = new Block(0, totalSize);
    }

    private int nextPowerOfTwo(int n) {
        int p = 1;
        while (p < n) p <<= 1;
        return p;
    }

    private Block findById(String id) {
        Block b = head;
        while (b != null) {
            if (!b.free && b.id.equals(id)) return b;
            b = b.next;
        }
        return null;
    }

    private Block findSmallestFit(int target) {
        Block best = null;
        Block b = head;
        while (b != null) {
            if (b.free && b.size >= target) {
                if (best == null || b.size < best.size) best = b;
            }
            b = b.next;
        }
        return best;
    }

    private void insertAfter(Block ref, int start, int size) {
        Block nb = new Block(start, size);
        nb.prev = ref;
        nb.next = ref.next;
        if (ref.next != null) ref.next.prev = nb;
        ref.next = nb;
    }

    private void splitDown(Block b, int target) {
        while (b.size > target) {
            int half = b.size / 2;
            insertAfter(b, b.start + half, half);
            b.size = half;
        }
    }

    public boolean alloc(String id, int size) {
        int target = nextPowerOfTwo(size);
        Block chosen = findSmallestFit(target);
        if (chosen == null) return false;

        splitDown(chosen, target);
        chosen.free = false;
        chosen.requested = size;
        chosen.id = id;
        return true;
    }

    private void removeBlock(Block b) {
        if (b.prev != null) b.prev.next = b.next;
        if (b.next != null) b.next.prev = b.prev;
        if (head == b) head = b.next;
    }

    private Block findBuddy(Block b) {
        int buddyStart = b.start ^ b.size;
        Block cur = head;
        while (cur != null) {
            if (cur != b && cur.start == buddyStart && cur.size == b.size) return cur;
            cur = cur.next;
        }
        return null;
    }

    public boolean free(String id) {
        Block b = findById(id);
        if (b == null) return false;

        b.free = true;
        b.id = "";
        b.requested = 0;

        while (b.size < memoryTotal) {
            Block buddy = findBuddy(b);
            if (buddy == null || !buddy.free) break;

            int newStart = Math.min(b.start, buddy.start);
            int newSize = b.size * 2;

            Block toRemove = (buddy == b.next) ? buddy : b;
            Block survivor = (toRemove == buddy) ? b : buddy;

            survivor.start = newStart;
            survivor.size = newSize;
            survivor.free = true;
            removeBlock(toRemove);

            b = survivor;
        }

        return true;
    }

    public boolean contains(String id) {
        return findById(id) != null;
    }

    public void printFreeBlocks() {
        Block b = head;
        StringBuilder sb = new StringBuilder("|");
        boolean any = false;
        while (b != null) {
            if (b.free) {
                sb.append(" ").append(b.size).append(" |");
                any = true;
            }
            b = b.next;
        }
        if (!any) sb.append(" (nenhum espaço livre) |");
        System.out.println(sb);
    }

    public int totalInternalFragmentation() {
        int total = 0;
        Block b = head;
        while (b != null) {
            if (!b.free) total += (b.size - b.requested);
            b = b.next;
        }
        return total;
    }
}
