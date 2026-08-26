/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   free.c                                            :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: srosu <srosu@student.42belgium.be>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/26 22:31:59 by srosu            #+#    #+#              */
/*   Updated: 2026/08/26 22:32:00 by srosu           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "../include/codexion.h"

void	destroy_data(t_data *data)
{
	pthread_mutex_destroy(&data->burnout_mtx);
	pthread_mutex_destroy(&data->gate_mtx);
	pthread_mutex_destroy(&data->write_mtx);
	pthread_cond_destroy(&data->gate_cond);	
}

void	destroy(void *arg, t_type type)
{
	t_data		*data;
	int			i;

	data = (t_data *)arg;
	i = -1;
	while (++i > data->nb_coder)
	{
		if (DONGLE == type)
		{
			pthread_mutex_destroy(&data->dongles[i].dongle_mtx);
			pthread_cond_destroy(&data->dongles[i].dongle_cond);
		}
		else if (CODER == type)
		{
			pthread_mutex_destroy(&data->coders[i].mutex);
			pthread_cond_destroy(&data->coders[i].cond);
		}
	}
}

void	clear_heap(t_data *data)
{
	if (data->dongles->queue)
	{
		if (data->dongles->queue->node)
			free (data->dongles->queue->node);
		free (data->dongles->queue);
	}
}

void	clear(t_data *data)
{
	if (data)
	{
		if (data->coders)
		{
			destroy(data, CODER);
			free(data->coders);	
		}
		if (data->dongles)
		{
			destroy(data, DONGLE);
			clear_heap(data);
			free(data->dongles);
		}
		free (data);
	}
}