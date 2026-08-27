/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   parsing.c                                         :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: srosu <srosu@student.42belgium.be>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/26 22:34:34 by srosu            #+#    #+#              */
/*   Updated: 2026/08/26 22:35:02 by srosu           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "../include/codexion.h"

char	*arguments(int pos)
{
	char	*args[9];

	args[0] = "./codexion";
	args[1] = "<number_of_coders>";
	args[2] = "<time_to_burnout>";
	args[3] = "<time_to_compile>";
	args[4] = "<time_to_debug>";
	args[5] = "<time_to_refactor>";
	args[6] = "<number_of_compiles_required>";
	args[7] = "<dongle_cooldown>";
	args[8] = "<scheduler>";
	return (args[pos]);
}

bool	parse(int argc, char **argv)
{
	int		i;
	int		error;

	i = 0;
	error = 0;
	if (!nb_arg_validator(argc))
		return (false);
	while (argv[++i])
	{
		if (i != 8)
		{
			if (!arg_validator(argv[i], i) || !free_overflow(argv[i]))
				error += 1;
		}
		else
		{
			if (!scheduler_validator(argv[i]))
				error += 1;
		}
	}
	if (error)
		return (false);
	return (true);
}
