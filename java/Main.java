import java.io.*;
import java.util.Scanner;

public class Main {

    enum CmdType { IN, OUT, INVALID }

    static class Command {
        CmdType type = CmdType.INVALID;
        String id = "";
        int size = 0;
    }

    private static final Scanner scanner = new Scanner(System.in);

    private static int readInt(String prompt) {
        while (true) {
            System.out.print(prompt);
            String line = scanner.nextLine().trim();
            try {
                return Integer.parseInt(line);
            } catch (NumberFormatException e) {
                System.out.println("Entrada inválida, tente novamente.");
            }
        }
    }

    private static boolean isPowerOfTwo(int n) {
        return n > 0 && (n & (n - 1)) == 0;
    }

    private static Command parseLine(String rawLine) {
        Command cmd = new Command();
        String line = rawLine.trim();
        if (line.isEmpty()) return cmd;

        if (line.startsWith("IN(")) {
            String content = line.substring(3);
            if (content.endsWith(")")) content = content.substring(0, content.length() - 1);
            String[] parts = content.split(",");
            if (parts.length == 2) {
                try {
                    cmd.type = CmdType.IN;
                    cmd.id = parts[0].trim();
                    cmd.size = Integer.parseInt(parts[1].trim());
                } catch (NumberFormatException e) {
                    cmd.type = CmdType.INVALID;
                }
            }
        } else if (line.startsWith("OUT(")) {
            String content = line.substring(4);
            if (content.endsWith(")")) content = content.substring(0, content.length() - 1);
            cmd.type = CmdType.OUT;
            cmd.id = content.trim();
        }

        return cmd;
    }

    public static void main(String[] args) throws IOException {
        System.out.println("==============================================");
        System.out.println(" Trabalho Prático 2 - Gerenciamento de Memória");
        System.out.println("==============================================\n");

        int memSize;
        do {
            memSize = readInt("Informe o tamanho da memória principal (potência de 2): ");
            if (!isPowerOfTwo(memSize)) {
                System.out.println("O tamanho deve ser uma potência de 2. Tente novamente.");
            }
        } while (!isPowerOfTwo(memSize));

        System.out.println("\nEscolha o tipo de gerenciamento de memória:");
        System.out.println("  1 - Partições Variáveis");
        System.out.println("  2 - Sistema Buddy");
        int mode;
        do {
            mode = readInt("Opcao: ");
            if (mode != 1 && mode != 2) System.out.println("Opção inválida. Digite 1 ou 2.");
        } while (mode != 1 && mode != 2);

        boolean useBuddy = (mode == 2);
        PartitionMemory partition = null;
        BuddyMemory buddy = null;

        if (!useBuddy) {
            System.out.println("\nEscolha a política de alocação:");
            System.out.println("  1 - Worst-Fit");
            System.out.println("  2 - Circular-Fit");
            int pol;
            do {
                pol = readInt("Opcao: ");
                if (pol != 1 && pol != 2) System.out.println("Opção inválida. Digite 1 ou 2.");
            } while (pol != 1 && pol != 2);
            PartitionMemory.Policy policy = (pol == 2)
                ? PartitionMemory.Policy.CIRCULAR_FIT
                : PartitionMemory.Policy.WORST_FIT;
            partition = new PartitionMemory();
            partition.init(memSize, policy);
        } else {
            buddy = new BuddyMemory();
            buddy.init(memSize);
        }

        System.out.print("\nInforme o caminho do arquivo de requisições: ");
        String filename = scanner.nextLine().trim();

        BufferedReader fileReader;
        try {
            fileReader = new BufferedReader(new FileReader(filename));
        } catch (FileNotFoundException e) {
            System.err.println("Não foi possível abrir o arquivo '" + filename + "'.");
            return;
        }

        System.out.println("\n--- Estado inicial ---");
        if (useBuddy) buddy.printFreeBlocks();
        else partition.printFreeBlocks();

        String rawLine;
        int lineNumber = 0;

        while ((rawLine = fileReader.readLine()) != null) {
            lineNumber++;
            Command cmd = parseLine(rawLine);

            if (cmd.type == CmdType.INVALID) continue;

            System.out.print("\n--- Linha " + lineNumber + ": ");
            if (cmd.type == CmdType.IN) {
                System.out.println("IN(" + cmd.id + ", " + cmd.size + ") ---");
                if (cmd.size <= 0) {
                    System.out.println("Tamanho inválido: deve ser maior que zero.");
                } else {
                    boolean exists = useBuddy ? buddy.contains(cmd.id) : partition.contains(cmd.id);
                    if (exists) {
                        System.out.println("Processo '" + cmd.id + "' já está na memória.");
                    } else {
                        boolean ok = useBuddy
                            ? buddy.alloc(cmd.id, cmd.size)
                            : partition.alloc(cmd.id, cmd.size);
                        if (!ok) System.out.println("ESPAÇO INSUFICIENTE DE MEMÓRIA");
                    }
                }
            } else {
                System.out.println("OUT(" + cmd.id + ") ---");
                boolean ok = useBuddy
                    ? buddy.free(cmd.id)
                    : partition.free(cmd.id);
                if (!ok) System.out.println("Processo '" + cmd.id + "' não encontrado na memória.");
            }

            if (useBuddy) {
                buddy.printFreeBlocks();
                System.out.println("Fragmentação interna total no momento: "
                    + buddy.totalInternalFragmentation());
            } else {
                partition.printFreeBlocks();
            }
        }

        fileReader.close();

        if (useBuddy) {
            System.out.println("\n--- Fragmentação interna total ao final da execução: "
                + buddy.totalInternalFragmentation() + " ---");
        }

        System.out.println("\nProcessamento concluído.");
    }
}
