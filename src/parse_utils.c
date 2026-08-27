/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   parse_utils.c                                     :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: srosu <srosu@student.42belgium.be>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/26 22:34:34 by srosu            #+#    #+#              */
/*   Updated: 2026/08/26 22:35:02 by srosu           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "../include/codexion.h"

bool	nb_arg_validator(int argc)
{
	int		i;

	i = 0;
	if (argc == 9)
		return (true);
	else if (argc > 9)
		fprintf(stderr, "USAGE ERROR: Too many arguments were given\n"
			" 	expected 8");
	fprintf(stderr, "Error: missing argument(s):\n");
	i = argc - 1;
	while (++i < 9)
	{
		fprintf(stderr, "%s", arguments(i));
		if (i < 9)
			fprintf(stderr, " ");
	}
	fprintf(stderr, "\n");
	return (false);
}

bool	arg_validator(char *argv, int pos)
{
	int	i;

	i = -1;
	while (argv[++i])
	{
		if (!(argv[i] >= '0' && argv[i] <= '9'))
		{
			if (argv[i] == '-' || argv[i] == '+')
				fprintf(stderr, "USAGE ERROR %s INVALID : "
					"signs are not allowed, expected a positive integer\n",
					arguments(pos));
			else
				fprintf(stderr, "USAGE ERROR %s INVALID : "
					"must contain only digits\n", arguments(pos));
			return (false);
		}
	}
	if (pos == 1 && atoi(argv) < 1)
	{
		fprintf(stderr, "USAGE ERROR: %s must be at least 1\n",
			arguments(1));
		return (false);
	}
	return (true);
}

bool	free_overflow(char *argv)
{
	int			i;
	long long	nb;

	i = -1;
	nb = 0;
	while (argv[++i])
	{
		nb = (nb * 10) + argv[i] - '0';
		if (nb > 2147483647)
		{
			fprintf(stderr, "OVERFLOW DETECTED\n");
			return (false);
		}
	}
	return (true);
}

bool	scheduler_validator(char *argv)
{
	if (!strcmp(argv, "fifo"))
		return (true);
	else if (!strcmp(argv, "edf"))
		return (true);
	fprintf(stderr, "SCHEDULER '%s' DOES NOT EXIST", argv);
	return (false);
}
