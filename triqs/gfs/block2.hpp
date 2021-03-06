/*******************************************************************************
 *
 * TRIQS: a Toolbox for Research in Interacting Quantum Systems
 *
 * Copyright (C) 2012-2015 by M. Ferrero, O. Parcollet
 *
 * TRIQS is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * TRIQS is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * TRIQS. If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/
#pragma once
#include "./gf_classes.hpp"
#include "./meshes/discrete.hpp"
#include <iterator>

namespace triqs {
namespace gfs {

 using block_index2 = cartesian_product<block_index, block_index>;
 template <typename G> struct is_block_gf_or_view<G, 2> : is_gf_or_view<G, block_index2>{};

 // -------------------------------  aliases  --------------------------------------------------

 template <typename... T> using block2_gf = gf<block_index2, gf<T...>>;
 template <typename... T> using block2_gf_view = gf_view<block_index2, gf<T...>>;
 template <typename... T> using block2_gf_const_view = gf_const_view<block_index2, gf<T...>>;

 /// ---------------------------  hdf5 ---------------------------------

 template <typename Target> struct gf_h5_name<block_index2, Target, nothing> {
  static std::string invoke() { return "Block2Gf"; }
 };

 template <typename Target> struct gf_h5_rw<block_index2, Target, nothing, void> {

  static void write(h5::group gr, gf_const_view<block_index2, Target> g) {
   auto const &m0 = std::get<0>(g.mesh());
   auto const &m1 = std::get<1>(g.mesh());
   for (int i = 0; i < m0.size(); ++i)
    for (int j = 0; j < m1.size(); ++j) h5_write(gr, m0.domain().names()[i] + "_" + m1.domain().names()[j], g._data[i][j]);
   h5_write(gr, "block_names1", m0.domain().names());
   h5_write(gr, "block_names2", m1.domain().names());
  }

  template <typename G > static void read(h5::group gr, G&g) {
   auto block_names1 = h5::h5_read<std::vector<std::string>>(gr, "block_names1");
   auto block_names2 = h5::h5_read<std::vector<std::string>>(gr, "block_names2");
   auto m0 = gf_mesh<block_index>(block_names1);
   auto m1 = gf_mesh<block_index>(block_names2);
   g._mesh = gf_mesh<block_index2>({m0, m1});
   g._data.resize(m0.size());
   for (int i = 0; i < m0.size(); ++i) {
    g._data[i].resize(m1.size());
    for (int j = 0; j < m1.size(); ++j) h5_read(gr, m0.domain().names()[i] + "_" + m1.domain().names()[j], g._data[i][j]);
   }
  }
 };

 /// ---------------------------  data access  ---------------------------------

 template <typename Target>
 struct gf_data_proxy<block_index2, Target, void> : data_proxy_vector2<typename regular_type_if_exists_else_type<Target>::type> {
 };

 // -------------------------------   Factories  --------------------------------------------------

 template <typename Target> struct gf_data_factory<block_index2, Target, nothing> {
  using mesh_t = gf_mesh<block_index>;
  using gf_t = gf<block_index2, Target>;
  using gf_view_t = gf_view<block_index2, Target>;
  using aux_t = nothing;
  struct target_shape_t {};

  static typename gf_t::data_t make(mesh_t const &m, target_shape_t, aux_t) {
   auto s = m.size_of_components();
   std::vector<Target> v(s[1]);
   return std::vector<std::vector<Target>>(s[0], v);
  }
 };

 // -------------------------------   Free Factories for regular type  --------------------------------------------------

 // from a number and a gf to be copied
 template <typename... A> block2_gf<A...> make_block2_gf(int n, int p, gf<A...> const &g) {
  auto V = std::vector<gf<A...>>(p, g);
  auto V2 = std::vector<std::vector<gf<A...>>>(n, V);
  return {{n, p}, std::move(V2), nothing{}, nothing{}, nothing{}};
 }

 // from vector<tuple<string,string>>, vector<gf>
 template <typename... A> block2_gf<A...> make_block2_gf(std::vector<std::string> const& block_names1,
                                                         std::vector<std::string> const& block_names2,
                                                         std::vector<std::vector<gf<A...>>> V) {
  if (block_names1.size()  != V.size())
   TRIQS_RUNTIME_ERROR << "make_block2_gf(vector<string>, vector<string>>, vector<vector<gf>>): incompatible outer vector size!";
  for(auto const& v : V) {
   if (block_names2.size()  != v.size())
    TRIQS_RUNTIME_ERROR << "make_block2_gf(vector<string>, vector<string>>, vector<vector<gf>>): incompatible inner vector size!";
  }
  return {{block_names1, block_names2}, std::move(V), nothing{}, nothing{}, nothing{}};
 }


 // -------------------------------   Free Factories for view type  --------------------------------------------------

 // from block_names and data vector
 template <typename GF>
 gf_view<block_index2, typename GF::regular_type> make_block2_gf_view(std::vector<std::string> block_names1,
                                                                      std::vector<std::string> block_names2,
                                                                      std::vector<std::vector<GF>> V) {
  return {{std::move(block_names1), std::move(block_names2)}, std::move(V), nothing{}, nothing{}, nothing{}};
 }

 // -------------------------------   Free functions   --------------------------------------------------

 /// The number of blocks
 template <typename G> TYPE_ENABLE_IF(int, is_block_gf_or_view<G, 2>) n_blocks(G const &g) { return g.mesh().size(); }
}
}

