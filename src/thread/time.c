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
#include <pthread.h>
#include <stdbool.h>
#include <time.h>

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

static bool	cooldown_left(t_data *data, t_dongle *d, struct timespec *ts)
{
	long long	rem;
	long long	target;

	if (d->last_release < 0)
		return (false);
	rem = data->cooldown - (timestamp(data) - d->last_release);
	if (rem <= 0)
		return (false);
	target = get_time() + rem;
	ts->tv_sec = target / 1000;
	ts->tv_nsec = (target % 1000) * 1000000;
	return (true);
}

void	wait_for_dongle(t_data *data, t_dongle *blocker, t_dongle *other)
{
	struct timespec	ts;

	pthread_mutex_unlock(&other->dongle_mtx);
	if (cooldown_left(data, blocker, &ts))
		pthread_cond_timedwait(&blocker->dongle_cond,
			&blocker->dongle_mtx, &ts);
	else
		pthread_cond_wait(&blocker->dongle_cond, &blocker->dongle_mtx);
	pthread_mutex_unlock(&blocker->dongle_mtx);
}
