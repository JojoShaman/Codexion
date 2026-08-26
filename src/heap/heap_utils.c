/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   heap_utils.c                                      :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: srosu <srosu@student.42belgium.be>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/26 22:34:34 by srosu            #+#    #+#              */
/*   Updated: 2026/08/26 22:35:02 by srosu           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/codexion.h"
#include <pthread.h>

void	push_node(t_heap *heap, t_coder *coder, long long deadline)
{
	heap->size++;
	heap->node[heap->size - 1].coder_id = coder->id;
	heap->node[heap->size - 1].deadline = deadline;
	heap->node[heap->size - 1].arrival = get_time();
	heap->node[heap->size - 1].active = true;
	sift_up(heap, heap->size - 1);
}

t_request	pop_node(t_heap *heap)
{
	t_request	ret;
	t_request	none;

	none.active = false;
	if (heap->size <= 0)
		return (none);
	ret = heap->node[0];
	heap->node[0] = heap->node[heap->size - 1];
	heap->size--;
	sift_down(heap, 0);
	return (ret);
}

void	remove_node(t_heap *heap, int coder_id)
{
	int	index;

	index = -1;
	if (heap->size == 0)
		return ;
	while (++index < heap->size)
	{
		if (heap->node[index].coder_id == coder_id)
			break ;
	}
	if (index >= heap->size)
		return ;
	heap->node[index] = heap->node[heap->size - 1];
	heap->size--;
	if (index < heap->size)
	{
		sift_up(heap, index);
		sift_down(heap, index);
	}
}

void	remove_dongles(t_dongle *first, t_dongle *second, t_coder *coder)
{
	pthread_mutex_lock(&first->dongle_mtx);
	pthread_mutex_lock(&second->dongle_mtx);
	remove_node(first->queue, coder->id);
	remove_node(second->queue, coder->id);
	pthread_mutex_unlock(&first->dongle_mtx);
	pthread_mutex_unlock(&second->dongle_mtx);
}

void	push_dongles(t_dongle *first, t_dongle *second, t_coder *coder)
{
	long long	last;

	pthread_mutex_lock(&coder->mutex);
	last = coder->last_compile;
	pthread_mutex_unlock(&coder->mutex);
	pthread_mutex_lock(&first->dongle_mtx);
	pthread_mutex_lock(&second->dongle_mtx);
	push_node(first->queue, coder,
		last + coder->data->time_to_burnout);
	push_node(second->queue, coder,
		last + coder->data->time_to_burnout);
	pthread_mutex_unlock(&first->dongle_mtx);
	pthread_mutex_unlock(&second->dongle_mtx);
}
