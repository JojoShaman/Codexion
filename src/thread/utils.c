/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   utils.c                                           :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: srosu <srosu@student.42belgium.be>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/26 22:34:34 by srosu            #+#    #+#              */
/*   Updated: 2026/08/26 22:35:02 by srosu           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/codexion.h"

bool is_burnout(t_data *data)
{
	bool ret;

	ret = false;
	pthread_mutex_lock(&data->burnout_mtx);
	if (data->burnout)
		ret = true;
	pthread_mutex_unlock(&data->burnout_mtx);
	return (ret);
}

void	coder_finished(t_data *data)
{
	pthread_mutex_lock(&data->gate_mtx);
	data->finished++;
	pthread_mutex_unlock(&data->gate_mtx);
}

bool is_end(t_data *data)
{
	bool done;

	pthread_mutex_lock(&data->gate_mtx);
	done = (data->finished == data->nb_coder);
	pthread_mutex_unlock(&data->gate_mtx);
	if (done)
		return (true);
	else if (is_burnout(data))
		return (true);
	return (false);

}

void	set_burnout(t_data *data)
{
	pthread_mutex_lock(&data->burnout_mtx);
	data->burnout = true;
	pthread_mutex_unlock(&data->burnout_mtx);
}

bool deadline_missed(t_coder *coder)
{
	long long	last;
	int			compiled;

	pthread_mutex_lock(&coder->mutex);
	last = coder->last_compile;
	compiled = coder->nb_compiled;
	pthread_mutex_unlock(&coder->mutex);
	if (last + coder->data->time_to_burnout < get_time())
	{
		if (compiled == coder->data->compiles_required)
			return (false);
		pthread_mutex_lock(&coder->data->burnout_mtx);
		coder->data->burnout = true;
		pthread_mutex_unlock(&coder->data->burnout_mtx);
		return (true);
	}
	return (false);
}
