#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <queue>
#include <cassert>
#include <set>
#include "CImg.h"

using namespace cimg_library;

struct event
{
	std::string name;
	std::pair<int, int> begin;
	std::pair<int, int> end;
	std::vector<std::string> prereqs;
	std::vector<std::string> groups;
};

// Check if event b is a prerequisite of event a
bool is_prereq(event a, event b)
{
	for (int i = 0; i < a.prereqs.size(); i++)
	{
		std::string cur_prereq = a.prereqs[i];
		if (cur_prereq == b.name)
		{
//			std::cout << std::endl;
//			std::cout << b.name << " is a direct prerequisite of " << a.name << std::endl;
//			system("pause");
			return true;
		}
		for (int j = 0; j < b.groups.size(); j++)
		{
			std::string cur_group = b.groups[j];
			if (cur_prereq == cur_group)
			{
//				std::cout << std::endl;
//				std::cout << b.name << " is a prerequisite of " << a.name << " by virtue of being a member of group " << cur_group << std::endl;
//				system("pause");
				return true;
			}
		}
	}
	return false;
}

struct graph_node
{
	std::pair<int, int> loc;
	std::vector<std::string> groups;
	std::map<std::string, int> costs;
};

struct radiator
{
	std::pair<int, int> loc;
	int id;
};

struct new_node
{
	std::vector<std::pair<std::pair<int, int>, int>> edges;
};

int main()
{
	CImg<unsigned char> gamemap("map.bmp");

	/*
	int WIDTH = gamemap.width();
	int HEIGHT = gamemap.height();

	bool** mask = new bool*[WIDTH];
	for (int i = 0; i < WIDTH; i++)
	{
		bool* row = new bool[HEIGHT];
		memset(row, true, sizeof(bool) * HEIGHT);
	}
	*/

	const int WIDTH = 1216;
	const int HEIGHT = 738;

	bool** mask = new bool* [WIDTH];
	for (int i = 0; i < WIDTH; i++)
	{
		bool* row = new bool[HEIGHT];
		mask[i] = row;
		for (int j = 0; j < HEIGHT; j++)
			mask[i][j] = true;
	}
	
	/*
	bool mask[WIDTH][HEIGHT];
	memset(mask, true, sizeof(bool) * WIDTH * HEIGHT);
	*/

	std::cout << "made mask" << std::endl;

	std::vector<event> events;

	for (int i = 0; i < WIDTH; i++)
		for (int j = 0; j < HEIGHT; j++)
			if (gamemap(i, j, 0, 0))
			{
			//	std::cout << "White pixel @ " << i << "," << j << std::endl;
				mask[i][j] = false;
			}
				

	std::cout << "set mask" << std::endl;

	std::map<std::string, int> name_to_num;

	std::ifstream infile("events.csv");
	std::string line;

	int i = 0;

	while (getline(infile, line))
	{
		std::cout << "Loading event #" << i << std::endl;
		std::stringstream ss(line);
		std::vector<std::string> curline;
		std::string tmp;
		while (getline(ss, tmp, ','))
			curline.push_back(tmp);

		event cur_event;
		cur_event.name = curline[0];
		cur_event.begin = std::pair<int, int>(stoi(curline[1]), stoi(curline[2]));
		cur_event.end = std::pair<int, int>(stoi(curline[3]), stoi(curline[4]));

		//all prereqs
		std::stringstream ss_preq(curline[5]);
		std::string tmp_preq;
		while (getline(ss_preq, tmp_preq, ';'))
		{
			std::cout << tmp_preq << " is a prereq of " << cur_event.name << std::endl;
			cur_event.prereqs.push_back(tmp_preq);
		}

		//all groups
		std::stringstream ss_group(curline[6]);
		std::string tmp_group;
		while (getline(ss_group, tmp_group, ';'))
		{
			std::cout << cur_event.name << " is a member of group " << tmp_group << std::endl;
			cur_event.groups.push_back(tmp_group);
//			system("pause");
		}
			

		name_to_num[cur_event.name] = i;
		events.push_back(cur_event);

		i++;
	}

	std::set<std::pair<int, int>> blocks;
	std::ifstream block_file("blocks.csv");
	std::string blockline;
	while (getline(block_file, blockline))
	{
		std::stringstream blss(blockline);
		std::string start;
		std::string end;
		getline(blss, start, ',');
		getline(blss, end, ',');
		int start_num = name_to_num[start];
		int end_num = name_to_num[end];
		blocks.insert(std::pair<int, int>(start_num, end_num));
		std::cout << start << "," << end << std::endl;
		std::cout << start_num << "," << end_num << std::endl;
	}
//	system("pause");

	std::ifstream subgraph("subgraph.csv");
	std::string graph_line;

	getline(subgraph, graph_line);
	int num_graph_const = stoi(graph_line);

	std::map<std::string, int> graph_const;

	for (int i = 0; i < num_graph_const; i++)
	{
		std::cout << "Loading graph constant #" << i << std::endl;
		getline(subgraph, graph_line);
		std::stringstream ss_graph(graph_line);
		std::string const_ss_name;
		std::string const_ss_val;

		getline(ss_graph, const_ss_name, ' ');
		getline(ss_graph, const_ss_val, ' ');

		graph_const[const_ss_name] = stoi(const_ss_val);
	}

	// process subgraph data
	std::vector<graph_node> nodes;
	
	while (getline(subgraph, graph_line))
	{
		std::stringstream cur_node(graph_line);
		std::vector<std::string> node_line;
		std::string tmp_node;
		while (getline(cur_node, tmp_node, ','))
			node_line.push_back(tmp_node);

		graph_node new_node;
		new_node.loc = std::pair<int, int>(stoi(node_line[0]), stoi(node_line[1]));
		//std::cout << new_node.loc.first << "," << new_node.loc.second << std::endl;

		//groups
		std::stringstream node_groups(node_line[2]);
		std::string ngroup_tmp;
		while (getline(node_groups, ngroup_tmp, ';'))
			new_node.groups.push_back(ngroup_tmp);

		//costs
		std::cout << node_line.size() << std::endl;
		std::stringstream node_costs(node_line[3]);
		std::string ncost_tmp;
		while (getline(node_costs, ncost_tmp, ';'))
		{
			std::stringstream this_cost(ncost_tmp);
			std::string cost_key;
			std::string cost_val;
			getline(this_cost, cost_key, ' ');
			getline(this_cost, cost_val, ' ');
			new_node.costs[cost_key] = graph_const[cost_val];
		}

		nodes.push_back(new_node);

	}

	std::map<std::pair<int, int>, std::vector<new_node>> new_nodes;

	for (int i = 0; i < nodes.size(); i++)
	{
		graph_node cur_node = nodes[i];
		for (int j = 0; j < nodes.size(); j++)
		{
			if (cur_node.costs.size() == 0) continue;
			graph_node dest_node = nodes[j];
			new_node node;
			for (int k = 0; k < dest_node.groups.size(); k++)
			{
				std::string cur_group = dest_node.groups[k];
				if (cur_node.costs.count(cur_group))
				{
					std::pair<std::pair<int, int>, int> n;
					n.first = dest_node.loc;
					n.second = cur_node.costs[cur_group];
					node.edges.push_back(n);
				}
			}
			if (new_nodes.count(cur_node.loc) == 0)
				new_nodes[cur_node.loc] = std::vector<new_node>();
			new_nodes[cur_node.loc].push_back(node);
		}
	}

	//heatmap needs distinct radiators from the beginning/end of each event, so 2 per event
	int N = events.size();
	std::cout << "N = " << N << std::endl;

	float*** heatmap = new float** [WIDTH];
	for (int i = 0; i < WIDTH; i++)
	{
		heatmap[i] = new float* [HEIGHT];
		for (int j = 0; j < HEIGHT; j++)
		{
			heatmap[i][j] = new float[2 * N];
			for (int k = 0; k < 2 * N; k++)
				heatmap[i][j][k] = 999999;
		}
	}

	std::queue<radiator> q;
	for (int i = 0; i < N; i++)
	{
		radiator r_start;
		r_start.id = 2 * i;
		r_start.loc = events[i].begin;
		q.push(r_start);
		heatmap[r_start.loc.first][r_start.loc.second][r_start.id] = 0;

		radiator r_end;
		r_end.id = 2 * i + 1;
		r_end.loc = events[i].end;
		q.push(r_end);
		heatmap[r_end.loc.first][r_end.loc.second][r_end.id] = 0;
	}

	const std::pair<int, int> DIRECT_OFFSETS[] =
	{
		std::pair<int,int>(0,-1),
		std::pair<int,int>(0, 1),
		std::pair<int,int>(-1, 0),
		std::pair<int,int>(1, 0)
	};

	const std::pair<int, int> DIAGON_OFFSETS[] =
	{
		std::pair<int,int>(-1,-1),
		std::pair<int,int>(1,-1),
		std::pair<int,int>(-1, 1),
		std::pair<int,int>(1, 1)
	};

	while (!q.empty())
	{
		radiator r = q.front();
		q.pop();
		int r_id = r.id;
		int r_x = r.loc.first;
		int r_y = r.loc.second;

		if (r_x == 0) continue;
//		std::cout << "Radiating ID " << r_id << " @ " << r_x << ", " << r_y << std::endl;
		if (mask[r_x][r_y])
		{
			std::cout << r_x << "," << r_y << std::endl;
		}
		assert(mask[r_x][r_y] == false);
		float r_heat = heatmap[r_x][r_y][r_id];

		float d_heat;

		if (new_nodes.count(r.loc))
		{
//			std::cout << "Outward edges detected at " << r_x << "," << r_y << std::endl;
			std::vector<new_node> these_nodes = new_nodes[r.loc];
			for (int i = 0; i < these_nodes.size(); i++)
			{
				new_node this_node = these_nodes[i];
				std::vector<std::pair<std::pair<int, int>, int>> these_edges = this_node.edges;
				for (int j = 0; j < these_edges.size(); j++)
				{
					std::pair<std::pair<int, int>, int> this_edge = these_edges[j];
					std::pair<int, int> dest_loc = this_edge.first;
					int d_x = dest_loc.first;
					int d_y = dest_loc.second;
					int travel_cost = this_edge.second;
					d_heat = heatmap[d_x][d_y][r_id];
					if (d_heat == -1 || d_heat > r_heat + travel_cost)
					{
						heatmap[d_x][d_y][r_id] = r_heat + travel_cost;
						radiator r_des;
						r_des.id = r_id;
						r_des.loc = dest_loc;
						q.push(r_des);
					}
				}
			}
		}

		// direct radiation (cost = 1)
		for (int i = 0; i < 4; i++)
		{
			std::pair<int, int> direction = DIRECT_OFFSETS[i];
			int d_x = direction.first + r_x;
			int d_y = direction.second + r_y;
			if (!mask[d_x][d_y])
			{
//		std::cout << "Direct movement from " << r_x << "," << r_y << " to " << d_x << ", " << d_y << std::endl;
				d_heat = heatmap[d_x][d_y][r_id];
				if (d_heat == -1 || d_heat > r_heat + 1)
				{
					heatmap[d_x][d_y][r_id] = r_heat + 1;
					radiator r_new;
					r_new.id = r_id;
					r_new.loc = std::pair<int, int>(d_x, d_y);
//			std::cout << " Direct radiation: new radiator @ " << d_x << "," << d_y << ": current heat: " << r_heat + 1 << std::endl;
					q.push(r_new);
				}
			}
		}
	
		// diagonal radiation (cost = sqrt(2))
		for (int i = 0; i < 4; i++)
		{
			std::pair<int, int> direction = DIAGON_OFFSETS[i];
			int d_x = direction.first + r_x;
			int d_y = direction.second + r_y;
			if (!mask[d_x][d_y])
			{
				d_heat = heatmap[d_x][d_y][r_id];
				if (d_heat == -1 || d_heat > r_heat + std::sqrt(2))
				{
					heatmap[d_x][d_y][r_id] = r_heat + std::sqrt(2);
					//				std::cout << "Node " << r_id << " radiates to " << d_x << ", " << d_y << " with new cost " << r_heat + std::sqrt(2) << std::endl;
					radiator r_new;
					r_new.id = r_id;
					r_new.loc = std::pair<int, int>(d_x, d_y);
					//std::cout << " Diagonal radiation: new radiator @ " << d_x << "," << d_y << ": current heat: " << r_heat + std::sqrt(2) << std::endl;
					q.push(r_new);
				}
			}
		}
	}

	// 



	std::cout << "NAME: kiwami2.sop" << std::endl;
	std::cout << "TYPE: SOP" << std::endl;
	std::cout << "COMMENT:" << std::endl;
	std::cout << "DIMENSION: " << N + 1 << std::endl;
	std::cout << "EDGE_WEIGHT_TYPE: EXPLICIT" << std::endl;
	std::cout << "EDGE_WEIGHT_FORMAT: FULL_MATRIX" << std::endl;
	std::cout << "EDGE_WEIGHT_SECTION" << std::endl;
	std::cout << N + 1 << std::endl;
	for (int i = 0; i < N; i++)
	{
		event cur = events[i];
		int loc_x = cur.end.first;
		int loc_y = cur.end.second;
		if (i == 0)
			std::cout << std::setw(7) << 0 << " ";
		else
			std::cout << std::setw(7) << -1 << " ";
		for (int j = 1; j < N; j++)
		{
			std::pair<int, int> block = std::pair<int, int>(i,j);
			if (blocks.count(block))
				std::cout << std::setw(7) << 999999 << " ";
			else if (i == j)
				std::cout << std::setw(7) << 0 << " ";
			else if (is_prereq(cur, events[j]))
			{
//				std::cout << events[j].name << " is a prereq of " << cur.name << std::endl;
//				system("pause");
				std::cout << std::setw(7) << -1 << " ";
			}
			//else if (cur.prereq == events[j].name)
				
			else
			{
				// if event is a "TAXI" event, find the distance to the nearest taxi
				if (events[j].begin.first == 0)
				{
					float best = 99999;
					for (int k = 0; k < nodes.size(); k++)
					{
						graph_node cur_node = nodes[k];
						std::pair<int, int> node_loc = cur_node.loc;
						if (cur_node.costs.size() > 0)
							best = std::min(best, heatmap[node_loc.first][node_loc.second][2 * i + 1]);
					}
					std::cout << std::setw(7) << int(best) << " ";
					//std::cout << std::setw(7) << 5 << " ";

					/*
					if (best == 99999)
					{
						std::cout << std::endl;
						std::cout << "Could not find a path from event " << i << " to taxi: " << events[i].name << std::endl;
						system("pause");
					}
					*/
				}
				else
					std::cout << std::setw(7) << int(heatmap[loc_x][loc_y][2 * j]) << " ";
					//std::cout << std::setw(7) << 6 << " ";
			}

		}
		std::cout << std::setw(7) << 0 << std::endl;
	}

	std::cout << std::setw(7) << 0 << " ";
	for (int i = 0; i < N - 1; i++)
		std::cout << std::setw(7) << -1 << " ";
	std::cout << std::setw(7) << 0 << std::endl;

	std::cout << "EOF" << std::endl;

}

