/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   heap_create.c                                     :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: srosu <srosu@student.42belgium.be>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/26 22:34:34 by srosu            #+#    #+#              */
/*   Updated: 2026/08/26 22:35:02 by srosu           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/codexion.h"

void	*init_heap(t_data *data)
{
	t_heap	*heap;

	heap = malloc(sizeof(t_heap));
	if (!heap)
		return (NULL);
	heap->node = malloc(sizeof(t_request) * data->nb_coder);
	if (!heap->node)
	{
		free(heap);
		return (NULL);
	}
	heap->scheduler = data->scheduler;
	heap->size = 0;
	return (heap);
}

bool	priority(t_request first, t_request second, t_mode scheduler)
{
	if (scheduler == EDF)
	{
		if (first.deadline == second.deadline)
		{
			if (first.arrival == second.arrival)
				return (first.coder_id < second.coder_id);
			return (first.arrival < second.arrival);
		}
		return (first.deadline < second.deadline);
	}
	return (first.arrival < second.arrival);
}

void	sift_up(t_heap *heap, int index)
{
	int			parent;
	t_request	tmp;

	parent = (index - 1) / 2;
	while (index > 0 && priority(heap->node[index],
			heap->node[parent], heap->scheduler))
	{
		tmp = heap->node[parent];
		heap->node[parent] = heap->node[index];
		heap->node[index] = tmp;
		index = parent;
		parent = (index - 1) / 2;
	}
}

void	sift_down(t_heap *heap, int index)
{
	t_request	tmp;
	int			parent;
	int			child1;
	int			child2;

	parent = index;
	child1 = 2 * index + 1;
	child2 = 2 * index + 2;
	if (child1 < heap->size
		&& priority(heap->node[child1], heap->node[parent], heap->scheduler))
		parent = child1;
	if (child2 < heap->size
		&& priority(heap->node[child2], heap->node[parent], heap->scheduler))
		parent = child2;
	if (parent != index)
	{
		tmp = heap->node[index];
		heap->node[index] = heap->node[parent];
		heap->node[parent] = tmp;
		sift_down(heap, parent);
	}
}
