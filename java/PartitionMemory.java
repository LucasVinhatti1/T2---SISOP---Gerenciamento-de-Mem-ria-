public class PartitionMemory {

    enum Policy { WORST_FIT, CIRCULAR_FIT }

    private Block head;
    private Policy policy;
    private Block circularCursor;

    public void init(int totalSize, Policy p) {
        head = new Block(0, totalSize);
        this.policy = p;
        this.circularCursor = head;
    }

    private Block findById(String id) {
        Block b = head;
        while (b != null) {
            if (!b.free && b.id.equals(id)) return b;
            b = b.next;
        }
        return null;
    }

    private Block newBlockAfter(Block ref, int start, int size) {
        Block nb = new Block(start, size);
        nb.prev = ref;
        nb.next = ref.next;
        if (ref.next != null) ref.next.prev = nb;
        ref.next = nb;
        return nb;
    }

    private void splitBlock(Block b, int size) {
        if (b.size > size) {
            newBlockAfter(b, b.start + size, b.size - size);
            b.size = size;
        }
    }

    private Block worstFitSearch(int size) {
        Block best = null;
        Block b = head;
        while (b != null) {
            if (b.free && b.size >= size) {
                if (best == null || b.size > best.size) best = b;
            }
            b = b.next;
        }
        return best;
    }

    private Block circularFitSearch(int size) {
        if (head == null) return null;
        if (circularCursor == null) circularCursor = head;

        Block startNode = circularCursor;
        Block b = circularCursor;
        boolean wrappedOnce = false;

        do {
            if (b.free && b.size >= size) return b;
            b = b.next;
            if (b == null) {
                b = head;
                wrappedOnce = true;
            }
            if (wrappedOnce && b == startNode) break;
        } while (b != startNode);

        if (startNode.free && startNode.size >= size) return startNode;
        return null;
    }

    public boolean alloc(String id, int size) {
        Block chosen = (policy == Policy.WORST_FIT)
            ? worstFitSearch(size)
            : circularFitSearch(size);

        if (chosen == null) return false;

        splitBlock(chosen, size);
        chosen.free = false;
        chosen.id = id;

        if (policy == Policy.CIRCULAR_FIT) {
            circularCursor = (chosen.next != null) ? chosen.next : head;
        }

        return true;
    }

    private boolean mergeWithNextIfFree(Block b) {
        if (b == null || b.next == null) return false;
        if (b.free && b.next.free) {
            Block nxt = b.next;
            b.size += nxt.size;
            b.next = nxt.next;
            if (nxt.next != null) nxt.next.prev = b;
            if (circularCursor == nxt) circularCursor = b;
            return true;
        }
        return false;
    }

    public boolean free(String id) {
        Block b = findById(id);
        if (b == null) return false;

        b.free = true;
        b.id = "";

        Block cur = head;
        while (cur != null) {
            while (mergeWithNextIfFree(cur)) {}
            cur = cur.next;
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
}
