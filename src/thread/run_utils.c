/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   run_utils.c                                       :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: srosu <srosu@student.42belgium.be>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/26 22:34:34 by srosu            #+#    #+#              */
/*   Updated: 2026/08/26 22:35:15 by srosu           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/codexion.h"

void	assign_dongle(t_coder *coder, t_dongle **first, t_dongle **second)
{
	if (coder->id % 2)
	{
		*first = coder->left;
		*second = coder->right;
	}
	else
	{
		*first = coder->right;
		*second = coder->left;
	}
}

bool	dongle_is_ready(t_coder *coder, t_dongle *dongle)
{
	if (dongle->taken)
		return (false);
	if (dongle->last_release > 0
		&& timestamp(coder->data) - dongle->last_release
		< coder->data->cooldown)
		return (false);
	return (dongle->queue
		&& dongle->queue->size
		&& dongle->queue->node[0].coder_id == coder->id);
}

bool	dongles_acquired(t_coder *coder, t_dongle *first,
	t_dongle	*second)
{
	if (is_end(coder->data))
	{
		pthread_mutex_unlock(&first->dongle_mtx);
		pthread_mutex_unlock(&second->dongle_mtx);
		return (false);
	}
	first->taken = true;
	second->taken = true;
	pop_node(first->queue);
	pop_node(second->queue);
	display_dongle(coder, DONGLE1, first);
	display_dongle(coder, DONGLE2, second);
	pthread_mutex_unlock(&first->dongle_mtx);
	pthread_mutex_unlock(&second->dongle_mtx);
	return (true);
}

bool	take_dongles(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;

	assign_dongle(coder, &first, &second);
	if (first == second)
		return (false);
	push_dongles(first, second, coder);
	while (!is_end(coder->data))
	{
		pthread_mutex_lock(&first->dongle_mtx);
		pthread_mutex_lock(&second->dongle_mtx);
		if (dongle_is_ready(coder, first)
			&& dongle_is_ready(coder, second))
			return (dongles_acquired(coder, first, second));
		if (is_end(coder->data))
		{
			pthread_mutex_unlock(&first->dongle_mtx);
			pthread_mutex_unlock(&second->dongle_mtx);
			break ;
		}
		pthread_mutex_unlock(&first->dongle_mtx);
		pthread_mutex_unlock(&second->dongle_mtx);
	}
	remove_dongles(first, second, coder);
	return (false);
}

bool	release_dongles(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;

	assign_dongle(coder, &first, &second);
	if (first == second || is_end(coder->data))
		return (false);
	pthread_mutex_lock(&first->dongle_mtx);
	pthread_mutex_lock(&second->dongle_mtx);
	first->taken = false;
	second->taken = false;
	first->last_release = timestamp(coder->data);
	second->last_release = timestamp(coder->data);
	pthread_cond_broadcast(&first->dongle_cond);
	pthread_cond_broadcast(&second->dongle_cond);
	pthread_mutex_unlock(&first->dongle_mtx);
	pthread_mutex_unlock(&second->dongle_mtx);
	return (true);
}
