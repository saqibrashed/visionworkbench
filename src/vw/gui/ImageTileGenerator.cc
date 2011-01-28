// __BEGIN_LICENSE__
// Copyright (C) 2006-2010 United States Government as represented by
// the Administrator of the National Aeronautics and Space Administration.
// All Rights Reserved.
// __END_LICENSE__


#include <vw/gui/ImageTileGenerator.h>
#include <vw/FileIO/DiskImageResource.h>
#include <vw/Core/Debugging.h>

namespace vw { namespace gui {

ImageTileGenerator::ImageTileGenerator(std::string filename) :
  m_rsrc( DiskImageResource::open(filename) ) {
  vw_out() << "\t--> Loading image: " << filename << ".\n";
}


// This little template makes the code below much cleaner.
template <class PixelT>
boost::shared_ptr<ViewImageResource> do_image_tilegen(boost::shared_ptr<SrcImageResource> rsrc,
                                                      BBox2i tile_bbox,
                                                      int level, int num_levels) {
  ImageView<PixelT> tile(tile_bbox.width(), tile_bbox.height());
  rsrc->read(tile.buffer(), tile_bbox);
  ImageView<PixelT> reduced_tile = subsample(tile, (1 << ((num_levels-1) - level)));
  return boost::shared_ptr<ViewImageResource>( new ViewImageResource(reduced_tile) );
}

boost::shared_ptr<ViewImageResource> ImageTileGenerator::generate_tile(TileLocator const& tile_info) {

  // Compute the bounding box of the image and the tile that is being
  // requested.  The bounding box of the tile depends on the pyramid
  // level we are looking at.
  BBox2i image_bbox(0,0,m_rsrc->cols(),m_rsrc->rows());
  BBox2i tile_bbox = tile_to_bbox(this->tile_size(), tile_info.col,
                                  tile_info.row, tile_info.level, this->num_levels());

  // Check to make sure the image intersects the bounding box.  Print
  // an error to screen and return an empty tile if it does not.
  if (!image_bbox.intersects(tile_bbox)) {
    vw_out() << "WARNING in ImageTileGenerator: a tile was requested that doesn't exist.";
    ImageView<PixelGray<uint8> > blank_tile(this->tile_size()[0], this->tile_size()[1]);
    return boost::shared_ptr<ViewImageResource>( new ViewImageResource(blank_tile) );
  }

  // Make sure we don't access any pixels outside the image boundary
  // by cropping the tile to the image dimensions.
  tile_bbox.crop(image_bbox);

  switch (this->pixel_format()) {
  case VW_PIXEL_GRAY:
    if (this->channel_type() == VW_CHANNEL_UINT8) {
      return do_image_tilegen<PixelGray<uint8> >(m_rsrc, tile_bbox,
                                                 tile_info.level, this->num_levels());
    } else if (this->channel_type() == VW_CHANNEL_INT16) {
      return do_image_tilegen<PixelGray<int16> >(m_rsrc, tile_bbox,
                                                 tile_info.level, this->num_levels());
    } else if (this->channel_type() == VW_CHANNEL_UINT16) {
      return do_image_tilegen<PixelGray<uint16> >(m_rsrc, tile_bbox,
                                                  tile_info.level, this->num_levels());
    } else if (this->channel_type() == VW_CHANNEL_FLOAT32) {
      return do_image_tilegen<PixelGray<float> >(m_rsrc, tile_bbox,
                                                  tile_info.level, this->num_levels());
    } else {
      std::cout << "This platefile has a channel type that is not yet support by vwv.\n";
      std::cout << "Exiting...\n\n";
      exit(0);
  }
  break;

  case VW_PIXEL_GRAYA:
    if (this->channel_type() == VW_CHANNEL_UINT8) {
      return do_image_tilegen<PixelGrayA<uint8> >(m_rsrc, tile_bbox,
                                                  tile_info.level, this->num_levels());
    } else if (this->channel_type() == VW_CHANNEL_INT16) {
      return do_image_tilegen<PixelGrayA<int16> >(m_rsrc, tile_bbox,
                                                  tile_info.level, this->num_levels());
    } else if (this->channel_type() == VW_CHANNEL_UINT16) {
      return do_image_tilegen<PixelGrayA<uint16> >(m_rsrc, tile_bbox,
                                                   tile_info.level, this->num_levels());
    } else if (this->channel_type() == VW_CHANNEL_FLOAT32) {
      return do_image_tilegen<PixelGrayA<float> >(m_rsrc, tile_bbox,
                                                  tile_info.level, this->num_levels());
    } else {
      std::cout << "This image has a channel type that is not yet support by vwv.\n";
      std::cout << "Exiting...\n\n";
      exit(0);
      }

    break;

  case VW_PIXEL_RGB:
    if (this->channel_type() == VW_CHANNEL_UINT8) {
      return do_image_tilegen<PixelRGB<uint8> >(m_rsrc, tile_bbox,
                                                tile_info.level, this->num_levels());
    } else if (this->channel_type() == VW_CHANNEL_UINT16) {
      return do_image_tilegen<PixelRGB<uint16> >(m_rsrc, tile_bbox,
                                                tile_info.level, this->num_levels());
    } else {
      std::cout << "This image has a channel type that is not yet support by vwv.\n";
      std::cout << "Exiting...\n\n";
      exit(0);
    }

    break;

  case VW_PIXEL_RGBA:
    if (this->channel_type() == VW_CHANNEL_UINT8) {
      return do_image_tilegen<PixelRGBA<uint8> >(m_rsrc, tile_bbox,
                                                 tile_info.level, this->num_levels());
    } else if (this->channel_type() == VW_CHANNEL_UINT16) {
      return do_image_tilegen<PixelRGBA<uint16> >(m_rsrc, tile_bbox,
                                                  tile_info.level, this->num_levels());
    } else {
      std::cout << "This image has a channel type that is not yet support by vwv.\n";
      std::cout << "Exiting...\n\n";
      exit(0);
    }

    break;

  default:
    std::cout << "This image has a pixel format that is not yet support by vwv.\n";
    std::cout << "Exiting...\n\n";
    exit(0);
  }

  vw_throw(NoImplErr() << "Unsupported pixel format or channel type in TileGenerator.\n");
}

Vector2 ImageTileGenerator::minmax() {
  vw_throw(NoImplErr() << VW_CURRENT_FUNCTION << " not implemented.");
}

PixelRGBA<float32> ImageTileGenerator::sample(int /*x*/, int /*y*/, int /*level*/, int /*transaction_id*/) {
  vw_throw(NoImplErr() << VW_CURRENT_FUNCTION << " not implemented.");
}


int ImageTileGenerator::cols() const {
  return m_rsrc->cols();
}

int ImageTileGenerator::rows() const {
  return m_rsrc->rows();
}

PixelFormatEnum ImageTileGenerator::pixel_format() const {
  return m_rsrc->pixel_format();
}

ChannelTypeEnum ImageTileGenerator::channel_type() const {
  return m_rsrc->channel_type();
}

Vector2i ImageTileGenerator::tile_size() const {
  return m_rsrc->block_read_size();
}

int32 ImageTileGenerator::num_levels() const {
  int32 max_dimension = std::max(this->cols(), this->rows());
  int32 max_tilesize = std::max(this->tile_size()[0], this->tile_size()[1]);
  return 1 + boost::numeric_cast<int32>(ceil(log(float(max_dimension) / float(max_tilesize)) / log(2)));
}

}} // namespace vw::gui
