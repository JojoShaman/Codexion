/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   run.c                                             :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: srosu <srosu@student.42belgium.be>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/26 22:34:34 by srosu            #+#    #+#              */
/*   Updated: 2026/08/26 22:35:02 by srosu           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/codexion.h"

static void	wait_all_coders(t_data *data)
{
	pthread_mutex_lock(&data->gate_mtx);
	while (!data->coders_ready)
		pthread_cond_wait(&data->gate_cond,
			&data->gate_mtx);
	pthread_mutex_unlock(&data->gate_mtx);
}

static void	safe_broadcast(t_data *data)
{
	int	i;

	i = -1;
	while (++i < data->nb_coder)
	{
		pthread_mutex_lock(&data->coders[i].mutex);
		pthread_cond_broadcast(&data->coders[i].cond);
		pthread_mutex_unlock(&data->coders[i].mutex);
		pthread_mutex_lock(&data->dongles[i].dongle_mtx);
		pthread_cond_broadcast(&data->dongles[i].dongle_cond);
		pthread_mutex_unlock(&data->dongles[i].dongle_mtx);
	}
}

void	*run(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	wait_all_coders(coder->data);
	while (!is_end(coder->data))
	{
		if (coder->nb_compiled == coder->data->compiles_required)
		{
			coder_finished(coder->data);
			break ;
		}
		if (!take_dongles(coder))
			break ;
		if (!compile(coder))
		{
			release_dongles(coder);
			break ;
		}
		release_dongles(coder);
		debug_refactor(coder);
	}
	return (NULL);
}

void	*monitoring(void *arg)
{
	t_data	*data;
	t_coder	*current;
	int		i;

	data = (t_data *) arg;
	wait_all_coders(data);
	while (true)
	{
		i = -1;
		while (++i < data->nb_coder)
		{
			current = &data->coders[i];
			if (deadline_missed(current))
			{
				set_burnout(data);
				safe_broadcast(data);
				log_burnout(current);
				return (NULL);
			}
		}
		if (is_end(data))
			return (NULL);
		usleep(1000);
	}
	return (NULL);
}
