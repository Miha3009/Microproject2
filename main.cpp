#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <ctime>
using namespace std;

int n; // Количество каннибалов
int m; // Вместимость горшка
int food; // Текущее количество мяса в горшке
int food_for_cannibal; // Сколько за обед съедает один каннибал

sem_t sem_cook; // Семафор ожидания повара
pthread_mutex_t mutex_food; // Мьютекс для изменения food в каннибалах

void *Cannibal(void* param){
	int num = *((int*)param);

	for(int i = 0; i < food_for_cannibal; i++){
		// Изменение количества мяса может быть только под мьютексом
		pthread_mutex_lock(&mutex_food);

		// Если еды не осталось, зовем повара
		if(food == 0) {
			printf("Time %d. Cannibal %d woke the cook\n", (int)clock(), num);
			sem_post(&sem_cook);
		}

		while(food == 0); // Ждем пока повар приготовит еду
		// Переменная нужна для вывода. К моменту вывода кто-то еще может поесть
		int cur_food = --food;

		pthread_mutex_unlock(&mutex_food);

		printf("Time %d. Cannibal %d took a meat. There are %d meat in the pot\n",
				(int)clock(),  num, cur_food);
	}

	return nullptr;
}

void *Cook(void* param){
	while(true){
		// Повар ждет пока его не позовут
		sem_wait(&sem_cook);
		printf("Time %d. The cook woke up. There are %d meat in the pot\n",
				(int)clock(), food);
		int cur_food = food += m;
		printf("Time %d. The cook went to bed. There are %d meat in the pot\n",
				(int)clock(), cur_food);
	}
	// Можно заметить, что цикл бесконечный. Кто-то должен вручную закрыть этот поток
	return nullptr;
}

int main(int argc, char** argv) {
	if(argc < 4){
		printf("The program requires 3 arguments: the number of cannibals, ");
		printf("the pot capacity, the amount of meat for each cannibal");
		return 0;
	}

	n = atoi(argv[1]);
	m = atoi(argv[2]);
	food_for_cannibal = atoi(argv[3]);
	food = m;

	if(n <= 0 || m <= 0 || food_for_cannibal <= 0){
		printf("All arguments must be positive!");
		return 0;
	}

	printf("Time %d. The dinner started. There are %d meat in the pot\n", (int)clock(), food);

	sem_init(&sem_cook, 0, 0);
	pthread_mutex_init(&mutex_food, NULL);

	pthread_t thread_cook;
	pthread_create(&thread_cook, NULL, Cook, nullptr);

	// Каннибалы едят и в основном потоке, и в других потоках
	pthread_t thread_cannibals[n-1];
	int thread_nums[n];
	for(int i = 0; i < n - 1; i++){
		thread_nums[i] = i + 1;
		pthread_create(&thread_cannibals[i], NULL, Cannibal, (void*)&thread_nums[i]);
	}
	thread_nums[n-1] = n;
	Cannibal((void*)&thread_nums[n-1]);

	for(int i = 0; i < n - 1; i++){
		pthread_join(thread_cannibals[i], nullptr);
	}
	// Повару нужно сказать, чтобы он больше никого не ждал
	pthread_cancel(thread_cook);

	printf("Time %d. The dinner ended. There are %d meat in the pot", (int)clock(), food);

	return 0;
}
