/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   init.c                                            :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: srosu <srosu@student.42belgium.be>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/26 22:34:34 by srosu            #+#    #+#              */
/*   Updated: 2026/08/26 22:35:02 by srosu           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/codexion.h"

void create_thread(t_data * data, t_coder * coder, pthread_t * monitor,
	pthread_t	*thread)
{
	int	i;

	i = -1;
	while (++i < data->nb_coder)
	{
		coder[i].id = i + 1;
		coder[i].data = data;
		coder[i].last_compile = get_time();
		coder[i].nb_compiled = 0;
		coder[i].left = &data->dongles[(i + 1) % data->nb_coder];
		coder[i].right = &data->dongles[i];
		pthread_mutex_init(&coder[i].mutex, NULL);
		pthread_cond_init(&coder[i].cond, NULL);
	}
	pthread_create(monitor, NULL, monitoring, data);
	i = -1;
	while (++i < data->nb_coder)
		pthread_create(&thread[i],
		NULL, run, &coder[i]);
}

void	join_thread(t_data *data, pthread_t *thread, pthread_t *monitor)
{
	int	i;

	i = -1;
	while (++i < data->nb_coder)
	{
		pthread_join(thread[i], NULL);
	}
	pthread_join(*monitor, NULL);
}

void	*init_dongles(t_data *data)
{
	int			i;
	t_dongle	*dongles;

	i = -1;
	dongles = malloc(sizeof(t_dongle)*data->nb_coder);
	if (!dongles)
		return (NULL);
	while (++i < data->nb_coder)
	{
		dongles[i].id = i;
		dongles[i].last_release = -1;
		dongles[i].taken = false;
		dongles[i].queue = init_heap(data);
		pthread_cond_init(&dongles[i].dongle_cond, NULL);
		pthread_mutex_init(&dongles[i].dongle_mtx, NULL);
	}
	return (dongles);
}

bool init_simulation(t_data *data)
{
	t_coder		*coder;
	pthread_t	monitor;
	pthread_t	*thread;
	int			i;

	coder = malloc(sizeof(t_coder)*data->nb_coder);
	if (!coder)
		return (false);
	thread = malloc(sizeof(pthread_t)*data->nb_coder);
	if (!thread)
	{
		free(coder);
		return (false);
	}
	data->coders = coder;
	data->coders_ready = false;
	data->burnout = false;
	data->finished = 0;
	data->end = false;
	pthread_cond_init(&data->gate_cond, NULL);
	pthread_mutex_init(&data->gate_mtx, NULL);
	pthread_mutex_init(&data->write_mtx, NULL);
	pthread_mutex_init(&data->burnout_mtx, NULL);
	create_thread(data, coder, &monitor, thread);
	pthread_mutex_lock(&data->gate_mtx);
	data->start = get_time();
	i = -1;
	while (++i < data->nb_coder)
		coder[i].last_compile = data->start;
	data->coders_ready = true;
	pthread_cond_broadcast(&data->gate_cond);
	pthread_mutex_unlock(&data->gate_mtx);
	join_thread(data, thread, &monitor);
	// free (thread);
	return (true);

}
bool init_data(t_data * data, char * *argv)
{
	data->nb_coder = atoi(argv[1]);
	data->time_to_burnout = atoi(argv[2]);
	data->time_to_compile = atoi(argv[3]);
	data->time_to_debug = atoi(argv[4]);
	data->time_to_refactor = atoi(argv[5]);
	data->compiles_required = atoi(argv[6]);
	data->cooldown = atoi(argv[7]);
	data->scheduler = FIFO;
	if (!strcmp(argv[8], "edf"))
		data->scheduler = EDF;
	data->dongles = init_dongles(data);
	if (!data->dongles)
		return (false);
	return (true);
}
