/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ereali <ereali@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/10/18 05:32:46 by ereali            #+#    #+#             */
/*   Updated: 2021/11/12 03:17:52 by ereali           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

int	setdeath(t_data *data, t_philo **philo)
{
	int i;

	i = 0;
	while (data->dead == 0 && data->EOeat != data->nbphilo && data->meat != 0)
	{
		i = 0;
		while (i < data->nbphilo && data->dead == 0 && data->EOeat != data->nbphilo)
		{
			if ((ft_time() - (*philo)[i].data->stime) - (*philo)[i].feat >= data->die)
			{
				pthread_mutex_lock(&data->m_dead);
				// a proteger
				data->dead = 1;
				pthread_mutex_unlock(&data->m_dead);
				printf("%lld %d died\n", (ft_time() - data->stime), i + 1);
			}
			usleep(150);
			i++;
		}
	}
	return (0);
}

typedef void *(*t_routine)(void *);

void    *Routine(t_philo *philo)
{
	// tableau int pour avoir que f[2] test 2 90000 200 200 1
	int i;
	int f1;
	int f2;

	f1 = 0;
	f2 = 0;
	i = 0;
	if (pthread_mutex_lock(&philo->data->m_sync))
		return (NULL);
	if (pthread_mutex_unlock(&philo->data->m_sync))
		return (NULL);
	if (philo->philo_id % 2 == 1)
		usleep(150);
	while (philo->data->dead == 0 && philo->data->meat != 0 && philo->data->EOeat != philo->data->nbphilo)
	{
		if (philo->data->dead == 0 && !(philo->data->nbphilo == 1 && i >= 1) && philo->data->EOeat != philo->data->nbphilo)
			printf("%lld %d is thinking\n", (ft_time() - philo->data->stime), philo->philo_id);
		if (philo->philo_id % 2 == 0 && philo->data->dead == 0 && philo->data->EOeat != philo->data->nbphilo)
		{
			f2 = 1;
			f1 = 1;
			pthread_mutex_lock(&philo->mutex_fork);
			if (philo->data->dead == 0 && philo->data->EOeat != philo->data->nbphilo)
				printf("%lld %d has taken a fork\n", (ft_time() - philo->data->stime), philo->philo_id);
			pthread_mutex_lock(philo->mutex_fork2);
			// return (0); // fct prtege et destroy mutex
		}
		if ((philo->philo_id % 2 == 1 && philo->data->dead == 0) && !(philo->data->nbphilo == 1 && i >= 1))
		{
			f1 = 1;
			pthread_mutex_lock(philo->mutex_fork2);
			if (philo->data->dead == 0 && philo->data->EOeat != philo->data->nbphilo)
				printf("%lld %d has taken a fork\n", (ft_time() - philo->data->stime), philo->philo_id);
			if (philo->data->nbphilo != 1)
				{
				f2 = 1;
				pthread_mutex_lock(&philo->mutex_fork);
				}
			// return (0); // fct prtege et destroy mutex
		}
		if (f1 && f2)
			philo->feat = (ft_time() - philo->data->stime);
		if (philo->data->dead == 0 && f1 && f2 && philo->data->EOeat != philo->data->nbphilo)
		{
			printf("%lld %d has taken a fork\n", (ft_time() - philo->data->stime), philo->philo_id);
			printf("%lld %d is eating\n", (ft_time() - philo->data->stime), philo->philo_id);
			ft_msleep(&philo->data, philo->data->eat);
		}  // return (0); // fct prtege et destroy mutex
		if ((f1 && philo->data->nbphilo != 1) || (f1 && philo->data->dead))
			pthread_mutex_unlock(philo->mutex_fork2);
		if (f2)
			pthread_mutex_unlock(&philo->mutex_fork);
		if (philo->data->meat > 0 && (i + 1) == philo->data->meat && f1 && f2)
		{
			pthread_mutex_lock(&philo->data->m_EOeat);
			philo->data->EOeat += 1;
			pthread_mutex_unlock(&philo->data->m_EOeat);
		}
		if (philo->data->dead == 0 && (f1 && f2) && philo->data->EOeat != philo->data->nbphilo)
		{
			printf("%lld %d is sleeping\n", (ft_time() - philo->data->stime), philo->philo_id);
			ft_msleep(&philo->data,philo->data->sleep);
		}
		i++;

	}
	while (philo->data->dead == 0 && (philo->data->nbphilo == 1 && i >= 1))
	{
		usleep(150);
		//safe car fixe
	}
	return (philo);
}

int	philocreate(t_philo **philo)
{
	int i;

	i = 0;
	while (i < (*philo)->data->nbphilo)
	{
		if (pthread_create(&(*philo)[i].philosophe, NULL, (t_routine)Routine, &(*philo)[i]) != 0)
			return (-1);
		i++;
	}
	return (0);
}

int philojoin(t_philo **philo)
{
	int i;

	i = 0;
	while (i < (*philo)->data->nbphilo && pthread_join((*philo)[i].philosophe, NULL) == 0)
		i++;
	if(pthread_mutex_destroy(&(*philo)->data->m_sync))
		return (-1);
	if (i == (*philo)->data->nbphilo)
	 	return (0);
	else
		return (-1);
}

int main(int argc, char **argv)
{
	t_data  data;
	t_philo *philo;
	int i;

	i = 0;
	if (argc != 5 && argc != 6)
		return (0);
	data = init_data(argv);
	if (data.meat == 0 || data.nbphilo == 0)
		return (0);
	philo = init_philo(&data);
	if (philo == NULL)
		return (-1);
	if (pthread_mutex_lock(&data.m_sync) || philocreate(&philo))
		return(-2);
	if (pthread_mutex_unlock(&data.m_sync))
		return (-3);
	if (setdeath(&data, &philo) || philojoin(&philo))
		return(-4);
	free(philo);
	return (0);
}
