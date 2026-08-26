/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   time.c                                            :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: srosu <srosu@student.42belgium.be>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/26 22:34:34 by srosu            #+#    #+#              */
/*   Updated: 2026/08/26 22:35:02 by srosu           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/codexion.h"

long long	get_time(void)
{
	struct timeval	now;

	gettimeofday(&now, NULL);
	return (now.tv_sec * 1000 + now.tv_usec / 1000);
}

long long	timestamp(t_data *data)
{
	return (get_time() - data->start);
}

void	ft_usleep(t_coder *coder, int to_sleep)
{
	struct timeval	tv;
	struct timespec	ts;
	int				rt;

	gettimeofday(&tv, NULL);
	ts.tv_sec = tv.tv_sec + ((tv.tv_usec + (to_sleep * 1000)) / 1000000);
	ts.tv_nsec = ((tv.tv_usec + (to_sleep * 1000)) % 1000000) * 1000;
	while (!is_end(coder->data))
	{
		pthread_mutex_lock(&coder->mutex);
		rt = pthread_cond_timedwait(&coder->cond, &coder->mutex, &ts);
		pthread_mutex_unlock(&coder->mutex);
		if (rt == ETIMEDOUT)
			break ;
	}
}
