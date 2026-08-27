/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   log.c                                             :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: srosu <srosu@student.42belgium.be>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/26 22:34:34 by srosu            #+#    #+#              */
/*   Updated: 2026/08/26 22:35:02 by srosu           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/codexion.h"

void	log_routine(t_coder *coder, t_status status)
{
	t_data	*data;

	data = coder->data;
	if (is_end(data))
		return ;
	pthread_mutex_lock(&data->write_mtx);
	if (COMPILING == status)
		fprintf(stdout, "%llu %d is compiling\n", timestamp(data), coder->id);
	else if (DEBUGGING == status)
		fprintf(stdout, "%llu %d is debugging\n", timestamp(data), coder->id);
	else if (REFACTORING == status)
		fprintf(stdout, "%llu %d is refactoring\n", timestamp(data), coder->id);
	pthread_mutex_unlock(&data->write_mtx);
}

void	log_burnout(t_coder *coder)
{
	t_data	*data;

	data = coder->data;
	pthread_mutex_lock(&data->write_mtx);
	fprintf(stdout, "%llu %d burned out\n", timestamp(data), coder->id);
	pthread_mutex_unlock(&data->write_mtx);
}

void	log_dongle(t_coder *coder)
{
	t_data	*data;

	data = coder->data;
	if (is_end(data))
		return ;
	pthread_mutex_lock(&data->write_mtx);
	fprintf(stdout, "%llu %d has taken a dongle\n", timestamp(data),
		coder->id);
	pthread_mutex_unlock(&data->write_mtx);
}
