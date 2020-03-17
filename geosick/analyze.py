from dataclasses import dataclass
from typing import List, Optional, Tuple
from .geosick import Request, Response
from .interpolate import interpolate
from .meet import meet

def analyze(request: Request) -> Response:
    sick_samples, query_samples = request.sick_samples, request.query_samples
    if len(sick_samples) < 2:
        raise ArgumentError("There are not enough sick_samples")
    if  len(query_samples) < 2:
        raise ArgumentError("There are not enough query_samples")

    start_timestamp = max(sick_samples[0].timestamp_ms, query_samples[0].timestamp_ms)
    end_timestamp = min(sick_samples[-1].timestamp_ms, query_samples[-1].timestamp_ms)
    if start_timestamp >= end_timestamp:
        raise ArgumentError("sick_samples and query_samples do not intersect in time")

    ctx = Ctx()
    ctx.period_s = 30
    ctx.ne_origin = (sick_samples[0].latitude_e7, sick_samples[0].longitude_e7)
    timestamps = list(range(start_timestamp, end_timestamp, period*1000))
    sick_points = interpolate(ctx, sick_samples, timestamps)
    query_points = interpolate(ctx, query_samples, timestamps)
    response = meet(sick_points, query_points, period_s)
    return response
