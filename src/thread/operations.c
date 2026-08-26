/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   operations.c                                      :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: srosu <srosu@student.42belgium.be>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/26 22:34:34 by srosu            #+#    #+#              */
/*   Updated: 2026/08/26 22:35:02 by srosu           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/codexion.h"

bool	compile(t_coder *coder)
{
	pthread_mutex_lock(&coder->mutex);
	coder->last_compile = get_time();
	pthread_mutex_unlock(&coder->mutex);
	display(coder, COMPILING);
	ft_usleep(coder, coder->data->time_to_compile);
	if (is_end(coder->data))
		return (false);
	coder->nb_compiled++;
	return (true);
}

void	debug_refactor(t_coder *coder)
{
	display(coder, DEBUGGING);
	ft_usleep(coder, coder->data->time_to_debug);
	display(coder, REFACTORING);
	ft_usleep(coder, coder->data->time_to_refactor);
}
