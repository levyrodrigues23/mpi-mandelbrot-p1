#include <stdio.h>
#include <mpi.h>

#define NBTASKS 47
#define END -1

int tag    =110;
int tag_res=111;

int calcula(int i) {
   return( i ) ;
}

void mestre() {
   int task=0;
   int res;
   int sinal;
   int nb_slaves;
   int total_res = 0;
   MPI_Status stat;

   MPI_Comm_size(MPI_COMM_WORLD, &nb_slaves);
   nb_slaves--;
   /* Primeiro, manda uma tarefa para cada escravo */
   for (task=0 ; task<nb_slaves ; task++) {
      printf("Mestre manda tarefa %d para o escravo %d\n", task, task+1);
      MPI_Send(&task, 1, MPI_INT, task+1, tag, MPI_COMM_WORLD);
   }

   /* Depois, espero por resultados e manda mais uma tarefa para quem terminou
    */
   while (task < NBTASKS) {
      MPI_Recv(&res, 1, MPI_INT, MPI_ANY_SOURCE, tag_res, MPI_COMM_WORLD, &stat);
      total_res += res;
      printf("Mestre manda tarefa %d para o escravo %d\n", task, stat.MPI_SOURCE);
      MPI_Send(&task, 1, MPI_INT, stat.MPI_SOURCE, tag, MPI_COMM_WORLD);
      task++;
   }
   /* Sobram nb_slaves resultados para receber */
   for (task=0 ; task<nb_slaves ; task++) {
      MPI_Recv(&res, 1, MPI_INT, MPI_ANY_SOURCE, tag_res, MPI_COMM_WORLD, &stat);
      total_res += res;
      /* Manda uma mensagem de finalizacao */
      sinal = END;
      MPI_Send(&sinal, 1, MPI_INT, stat.MPI_SOURCE, tag, MPI_COMM_WORLD);
   }
   printf("O resultado vale %d\n", total_res);
   printf("Ele devia ser igual a %d\n.", (NBTASKS*(NBTASKS-1)/2));
}

void  escravo() {
   int task=0;
   int res;
   int r;
   MPI_Status stat;

   MPI_Comm_rank(MPI_COMM_WORLD, &r);
   while (task != END) {
      MPI_Recv(&task, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &stat);
      printf("Escravo %d: processando tarefa %d.\n", r, task);
      res = calcula(task);
      MPI_Send(&res, 1, MPI_INT, 0, tag_res, MPI_COMM_WORLD);
   }
   printf("Escravo %d acabando...\n", r);
}

int main(int argc, char* argv[]) {
   int p, r ;
   int tag = 103;
   MPI_Status stat;
   double valor ;

   MPI_Init(&argc, &argv) ;
   MPI_Comm_size(MPI_COMM_WORLD, &p);
   MPI_Comm_rank(MPI_COMM_WORLD, &r);

   if (r==0) {
      printf("Process 0 eh o mestre.\n");
      mestre();
   }
   else {
      printf("Process %d eh um escravo.\n", r);
      escravo();
   }
   MPI_Finalize();
   return 0 ;
}
