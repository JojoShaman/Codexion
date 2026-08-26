/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   codexion.h                                        :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: srosu <srosu@student.42belgium.be>        #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/08/26 22:34:34 by srosu            #+#    #+#              */
/*   Updated: 2026/08/26 22:35:15 by srosu           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */
#ifndef CODEXION_H
# define CODEXION_H

# include <unistd.h> // write
# include <stdlib.h> // malloc
# include <stdio.h> // ftprintf
# include <pthread.h> // thread tools
# include <string.h> // strlen, strcmp ...
# include <sys/time.h> // gettimeofday
# include <stdbool.h> // bool
# include <errno.h> // errors

typedef pthread_mutex_t	t_mtx;
typedef pthread_cond_t	t_cond;
typedef struct s_data	t_data;

typedef enum e_mode
{
	NONE,
	FIFO,
	EDF
}	t_mode;

typedef enum e_status
{
	DONGLE1,
	DONGLE2,
	COMPILING,
	DEBUGGING,
	REFACTORING,
	BURNEOUT
}	t_status;

typedef struct s_request
{
	int			coder_id;
	long long	deadline;
	long long	arrival;
	bool		active;
}	t_request;

typedef struct s_heap
{
	int			size;
	t_request	*node;
	t_mode		scheduler;
}	t_heap;

typedef struct s_dongle
{
	int			id;
	long long	last_release;
	bool		taken;
	t_heap		*queue;
	t_cond		dongle_cond;
	t_mtx		dongle_mtx;
}	t_dongle;

typedef struct s_coder
{
	int			id;
	int			nb_compiled;
	long long	last_compile;
	pthread_t	*thread;
	t_dongle	*right;
	t_dongle	*left;
	t_data		*data;
	t_mtx		mutex;
	t_cond		cond;
}	t_coder;

struct s_data
{
	int			nb_coder;
	int			time_to_burnout;
	int			time_to_compile;
	int			time_to_debug;
	int			time_to_refactor;
	int			compiles_required;
	int			cooldown;
	bool		burnout;
	bool		coders_ready;
	int			finished;
	bool		end;
	long long	start;
	t_dongle	*dongles;
	t_coder		*coders;
	t_mode		scheduler;
	t_cond		gate_cond;
	t_mtx		gate_mtx;
	t_mtx		write_mtx;
	t_mtx		burnout_mtx;
};

long long	get_time(void);
void		sift_up(t_heap *heap, int index);
void		sift_down(t_heap *heap, int index);
bool		parse(int argc, char **argv);
bool		init_simulation(t_data *data);
void		clean(t_data *data);
bool		init_data(t_data *data, char **argv);
void		debug_refactor(t_coder *coder);
void		push_dongles(t_dongle *first, t_dongle *second, t_coder *coder);
long long	timestamp(t_data *data);
void		*init_heap(t_data *data);
void		*monitoring(void *arg);
void		*run(void *arg);
void		push_node(t_heap *heap, t_coder *coder, long long deadline);
t_request	pop_node(t_heap *heap);
void		remove_node(t_heap *heap, int coder_id);
void		remove_dongles(t_dongle *first, t_dongle *second, t_coder *coder);
void		display(t_coder *coder, t_status status);
bool		is_end(t_data *data);
void		ft_usleep(t_coder *coder, int to_sleep);
t_request	find_node(t_heap *heap, int coder_id);
void		display_dongle(t_coder *coder, t_status status, t_dongle *dongle);
void		display_burnout(t_coder *coder);
void		cooldown(t_coder *coder, t_dongle *dongle);
bool		compile(t_coder *coder);
void		assign_dongle(t_coder *coder, t_dongle **first, t_dongle **second);
bool		dongle_is_ready(t_coder *coder, t_dongle *dongle);
bool		dongles_acquired(t_coder *coder, t_dongle *first, t_dongle *second);
bool		take_dongles(t_coder *coder);
bool		release_dongles(t_coder *coder);
bool		is_burnout(t_data *data);
void		coder_finished(t_data *data);
bool		deadline_missed(t_coder *coder);
void		set_burnout(t_data *data);
#endif